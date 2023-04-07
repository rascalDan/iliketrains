#include "faceController.h"
#include "collections.hpp"
#include "maths.h"
#include "modelFactoryMesh.h"
#include <glm/gtx/intersect.hpp>

void
FaceController::apply(ModelFactoryMesh & mesh, const StyleStack & parents, const std::string & names,
		Shape::CreatedFaces & faces) const
{
	std::stringstream nameStream {names};
	std::for_each(std::istream_iterator<std::string>(nameStream), std::istream_iterator<std::string> {},
			[&](const auto & name) {
				applySingle(mesh, parents, name, faces);
			});
}

std::string
FaceController::getAdjacentFaceName(
		const ModelFactoryMesh & mesh, const std::span<const OpenMesh::FaceHandle> ofrange, OpenMesh::FaceHandle nf)
{
	const auto nfrange = mesh.ff_range(nf);
	if (const auto target = std::find_first_of(ofrange.begin(), ofrange.end(), nfrange.begin(), nfrange.end());
			target != ofrange.end()) {
		return mesh.property(mesh.nameFaceProperty, *target);
	};
	return {};
};

void
FaceController::applySingle(ModelFactoryMesh & mesh, const StyleStack & parents, const std::string & name,
		Shape::CreatedFaces & faces) const
{
	auto controlledFaces {materializeRange(faces.equal_range(name))};

	if (!type.empty() || !splits.empty()) {
		faces.erase(name);
	}
	for (auto & [faceName, faceHandle] : controlledFaces) {
		Shape::CreatedFaces newFaces;
		for (const auto & [newFaceSuffix, splitDef] : splits) {
			newFaces.merge(split(mesh, name + newFaceSuffix, faceHandle, *splitDef));
		}
		if (type == "extrude") {
			newFaces.merge(extrude(mesh, name, faceHandle));
		}
		if (!newFaces.empty()) {
			applyStyle(mesh, parents + this, newFaces);
			for (const auto & [subFaceName, faceController] : faceControllers) {
				faceController->apply(mesh, parents + this, subFaceName, newFaces);
			}
			faces.merge(std::move(newFaces));
		}
		else {
			applyStyle(mesh, parents + this, faceHandle);
		}
	}
}

Shape::CreatedFaces
FaceController::extrude(ModelFactoryMesh & mesh, const std::string & faceName, OpenMesh::FaceHandle faceHandle) const
{
	// get points
	const auto baseVertices {materializeRange(mesh.fv_range(faceHandle))};
	auto points = std::accumulate(
			baseVertices.begin(), baseVertices.end(), std::vector<glm::vec3> {}, [&mesh](auto && out, auto && v) {
				out.push_back(mesh.point(v));
				return std::move(out);
			});
	const auto vertexCount = points.size();
	const auto centre = mesh.calc_face_centroid(faceHandle);
	Shape::CreatedFaces newFaces;
	// mutate points
	std::for_each(points.begin(), points.end(), [mutation = getMatrix(), &centre](auto && p) {
		p = centre + ((p - centre) % mutation);
	});
	// create new vertices
	std::vector<OpenMesh::VertexHandle> vertices;
	std::transform(points.begin(), points.end(), std::back_inserter(vertices), [&mesh](auto && p) {
		return mesh.add_vertex(p);
	});
	// create new faces
	const auto ofrange = materializeRange(mesh.ff_range(faceHandle));
	mesh.delete_face(faceHandle);
	for (size_t idx {}; idx < vertexCount; ++idx) {
		const auto next = (idx + 1) % vertexCount;
		const auto newFace = mesh.add_face({baseVertices[idx], baseVertices[next], vertices[next], vertices[idx]});
		auto & newFaceName = mesh.property(mesh.nameFaceProperty, newFace);
		newFaceName = getAdjacentFaceName(mesh, ofrange, newFace);
		newFaces.emplace(newFaceName, newFace);
	}
	newFaces.emplace(faceName, mesh.add_face(vertices));

	return newFaces;
}

enum class PlaneRelation { Above, Below, On };
Shape::CreatedFaces
FaceController::split(
		ModelFactoryMesh & mesh, const std::string & name, OpenMesh::FaceHandle & fh, const Split & split) const
{
	// Map face vertex handles to their relationship to the split plane
	const auto vertices = materializeRange(mesh.fv_range(fh));
	auto vertexRelations = vertices * [&split, &mesh](OpenMesh::VertexHandle vh) {
		const auto d = glm::dot(split.normal, mesh.point(vh) - split.origin);
		return std::make_pair(vh, d < 0.f ? PlaneRelation::Below : d > 0.f ? PlaneRelation::Above : PlaneRelation::On);
	};
	// Insert new vertices where half edges intersect the split plane
	for (size_t curIdx = 0; curIdx < vertexRelations.size(); ++curIdx) {
		const size_t nextIdx = (curIdx + 1) % vertexRelations.size();
		const auto &current = vertexRelations[curIdx], next = vertexRelations[nextIdx];
		if ((current.second == PlaneRelation::Above && next.second == PlaneRelation::Below)
				|| (current.second == PlaneRelation::Below && next.second == PlaneRelation::Above)) {
			const auto origin = mesh.point(current.first), dir = glm::normalize(mesh.point(next.first) - origin);

			float dist {};
			glm::intersectRayPlane(origin, dir, split.origin, split.normal, dist);
			const auto newv = mesh.add_vertex(origin + (dir * dist));
			auto where = vertexRelations.begin();
			++curIdx;
			std::advance(where, curIdx);
			vertexRelations.emplace(where, newv, PlaneRelation::On);
		}
	}
	// Create vertex vectors
	std::array<std::vector<OpenMesh::VertexHandle>, 2> out;
	auto filterVertices = [&vertexRelations](auto & out, auto notRelation) {
		for (const auto & vhr : vertexRelations) {
			if (vhr.second != notRelation) {
				out.emplace_back(vhr.first);
			}
		}
	};
	filterVertices(out.front(), PlaneRelation::Above);
	filterVertices(out.back(), PlaneRelation::Below);

	if (out.back().size() > 2) {
		Shape::CreatedFaces newFaces;
		const auto oldName = mesh.property(mesh.nameFaceProperty, fh);
		mesh.delete_face(fh);
		const auto newf1 = newFaces.insert(mesh.add_namedFace(oldName, out.front()))->second;
		const auto newf2 = newFaces.insert(mesh.add_namedFace(name, out.back()))->second;
		mesh.copy_property(mesh.smoothFaceProperty, fh, newf1);
		mesh.copy_property(mesh.smoothFaceProperty, fh, newf2);
		fh = newf1;
		return newFaces;
	}
	return {};
}

bool
FaceController::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_MEMBER(id) && Style::persist(store) && STORE_MEMBER(type) && Mutation::persist(store)
			&& STORE_NAME_HELPER("split", splits, Persistence::MapByMember<Splits>)
			&& STORE_NAME_HELPER("face", faceControllers, Persistence::MapByMember<FaceControllers>);
}

bool
FaceController::Split::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_MEMBER(id) && STORE_MEMBER(origin) && STORE_MEMBER(normal);
}
