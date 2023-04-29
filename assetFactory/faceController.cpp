#include "faceController.h"
#include "collections.h"
#include "maths.h"
#include "modelFactoryMesh.h"
#include "ray.h"

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
	// create new vertices
	const auto vertices
			= baseVertices * [&mesh, mutation = getMatrix(), centre = mesh.calc_face_centroid(faceHandle)](auto && v) {
				  return mesh.add_vertex(centre + ((mesh.point(v) - centre) % mutation));
			  };
	// get new faces names
	const auto vertexCount = baseVertices.size();
	std::vector<std::string> faceNames;
	for (size_t idx {}; idx < vertexCount; ++idx) {
		const auto next = (idx + 1) % vertexCount;
		const auto existingEdge = mesh.find_halfedge(baseVertices[idx], baseVertices[next]);
		faceNames.push_back(mesh.property(mesh.nameAdjFaceProperty, existingEdge));
	}
	// create new faces
	mesh.delete_face(faceHandle);
	Shape::CreatedFaces newFaces;
	for (size_t idx {}; idx < vertexCount; ++idx) {
		const auto next = (idx + 1) % vertexCount;
		newFaces.emplace(mesh.add_namedFace(
				faceNames[idx], baseVertices[idx], baseVertices[next], vertices[next], vertices[idx]));
	}
	newFaces.emplace(mesh.add_namedFace(faceName, vertices));

	return newFaces;
}

Shape::CreatedFaces
FaceController::split(
		ModelFactoryMesh & mesh, const std::string & name, OpenMesh::FaceHandle & fh, const Split & split) const
{
	// Map face vertex handles to their relationship to the split plane
	const auto vertices = materializeRange(mesh.fv_range(fh));
	auto vertexRelations = vertices * [&split, &mesh](OpenMesh::VertexHandle vh) {
		return std::make_pair(vh, split.getRelation(mesh.point(vh)));
	};
	// Insert new vertices where half edges intersect the split plane
	for (size_t curIdx = 0; curIdx < vertexRelations.size(); ++curIdx) {
		const size_t nextIdx = (curIdx + 1) % vertexRelations.size();
		const auto &current = vertexRelations[curIdx], next = vertexRelations[nextIdx];
		if (GeometricPlane::isIntersect(current.second, next.second)) {
			const auto ray = Ray::fromPoints(mesh.point(current.first), mesh.point(next.first));
			const auto intersect = split.getRayIntersectPosition(ray);
			assert(intersect);
			const auto newv = mesh.add_vertex(intersect->position);
			auto where = vertexRelations.begin();
			++curIdx;
			std::advance(where, curIdx);
			vertexRelations.emplace(where, newv, GeometricPlane::PlaneRelation::On);
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
	filterVertices(out.front(), GeometricPlane::PlaneRelation::Above);
	filterVertices(out.back(), GeometricPlane::PlaneRelation::Below);

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
