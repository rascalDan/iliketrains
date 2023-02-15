#include "faceController.h"
#include "assetFactoryConfig.h"
#include "maths.h"

void
FaceController::apply(ModelFactoryMesh & mesh, const std::string & name, Shape::CreatedFaces & faces) const
{
	if (!type.empty()) {
		const auto mutation = getMatrix();
		const auto controlledFacesRange = faces.equal_range(name);
		const std::vector controlledFaces(controlledFacesRange.first, controlledFacesRange.second);
		faces.erase(name);
		for (const auto & cf : controlledFaces) {
			// get face vertices
			const auto faceVertexRange = mesh.fv_range(cf.second);
			// get points
			const std::vector baseVertices(faceVertexRange.begin(), faceVertexRange.end());
			std::vector<glm::vec3> points;
			std::transform(
					faceVertexRange.begin(), faceVertexRange.end(), std::back_inserter(points), [&mesh](auto && v) {
						return mesh.point(v);
					});
			const auto vertexCount = points.size();
			const auto centre
					= std::accumulate(points.begin(), points.end(), glm::vec3 {}) / static_cast<float>(vertexCount);
			if (type == "extrude") {
				Shape::CreatedFaces newFaces;
				// mutate points
				std::for_each(points.begin(), points.end(), [&mutation, &centre](auto && p) {
					p = centre + ((p - centre) % mutation);
				});
				// create new vertices
				std::vector<OpenMesh::VertexHandle> vertices;
				std::transform(points.begin(), points.end(), std::back_inserter(vertices), [&mesh](auto && p) {
					return mesh.add_vertex({p.x, p.y, p.z});
				});
				// create new faces

				mesh.delete_face(cf.second);
				for (size_t idx {}; idx < vertexCount; ++idx) {
					const auto next = (idx + 1) % vertexCount;
					newFaces.emplace("extrusion",
							mesh.add_face({baseVertices[idx], baseVertices[next], vertices[next], vertices[idx]}));
				}
				newFaces.emplace(name, mesh.add_face(vertices));
				for (const auto & [name, faceController] : faceControllers) {
					faceController.apply(mesh, name, newFaces);
				}
				faces.merge(std::move(newFaces));
			}
		}
	}
}
