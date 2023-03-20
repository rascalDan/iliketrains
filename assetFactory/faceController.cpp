#include "faceController.h"
#include "collections.hpp"
#include "maths.h"
#include "modelFactoryMesh.h"

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
	const auto controlledFaces {materializeRange(faces.equal_range(name))};
	if (controlledFaces.empty()) {
		throw std::runtime_error("Named face(s) do not exist: " + name);
	}

	if (!type.empty()) {
		faces.erase(name);
	}
	for (const auto & [faceName, faceHandle] : controlledFaces) {
		if (type == "extrude") {
			auto newFaces = extrude(mesh, faceName, faceHandle);
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
	for (const auto & [faceName, faceHandle] : controlledFaces) {
		applyStyle(mesh, parents + this, faceHandle);
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

bool
FaceController::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_MEMBER(id) && Style::persist(store) && STORE_MEMBER(type) && Mutation::persist(store)
			&& STORE_NAME_HELPER("face", faceControllers, Persistence::MapByMember<FaceControllers>);
}
