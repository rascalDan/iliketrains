#include "faceController.h"
#include "collections.hpp"
#include "maths.h"
#include "modelFactoryMesh.h"

void
FaceController::apply(ModelFactoryMesh & mesh, const StyleStack & parents, const std::string & name,
		Shape::CreatedFaces & faces) const
{
	const auto getAdjacentFaceName = [&mesh](const auto & ofrange, OpenMesh::FaceHandle nf) -> std::string {
		const auto nfrange = mesh.ff_range(nf);
		if (const auto target = std::find_first_of(ofrange.begin(), ofrange.end(), nfrange.begin(), nfrange.end());
				target != ofrange.end()) {
			return mesh.property(mesh.nameFaceProperty, *target);
		};
		return {};
	};

	const auto controlledFaces {materializeRange(faces.equal_range(name))};
	if (controlledFaces.empty()) {
		throw std::runtime_error("Named face(s) do not exist: " + name);
	}

	if (!type.empty()) {
		const auto mutation = getMatrix();
		faces.erase(name);
		for (const auto & cf : controlledFaces) {
			// get points
			const auto baseVertices {materializeRange(mesh.fv_range(cf.second))};
			auto points = std::accumulate(baseVertices.begin(), baseVertices.end(), std::vector<glm::vec3> {},
					[&mesh](auto && out, auto && v) {
						out.push_back(mesh.point(v));
						return std::move(out);
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
				const auto ofrange = materializeRange(mesh.ff_range(cf.second));
				mesh.delete_face(cf.second);
				for (size_t idx {}; idx < vertexCount; ++idx) {
					const auto next = (idx + 1) % vertexCount;
					const auto newFace
							= mesh.add_face({baseVertices[idx], baseVertices[next], vertices[next], vertices[idx]});
					auto & name = mesh.property(mesh.nameFaceProperty, newFace);
					name = getAdjacentFaceName(ofrange, newFace);
					newFaces.emplace(name, newFace);
				}
				newFaces.emplace(name, mesh.add_face(vertices));
				if (smooth) {
					for (const auto & [name, face] : newFaces) {
						mesh.property(mesh.smoothFaceProperty, face) = true;
					}
				}
				applyStyle(mesh, parents + this, newFaces);
				for (const auto & [name, faceController] : faceControllers) {
					faceController->apply(mesh, parents + this, name, newFaces);
				}
				faces.merge(std::move(newFaces));
			}
			else {
				mesh.property(mesh.smoothFaceProperty, cf.second) = smooth;
				applyStyle(mesh, parents + this, cf.second);
			}
		}
	}
	else {
		for (const auto & cf : controlledFaces) {
			applyStyle(mesh, parents + this, cf.second);
		}
	}
}

bool
FaceController::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_MEMBER(id) && Style::persist(store) && STORE_MEMBER(type) && STORE_MEMBER(smooth)
			&& STORE_MEMBER(scale) && STORE_MEMBER(position) && STORE_MEMBER(rotation)
			&& STORE_NAME_HELPER("face", faceControllers, Persistence::MapByMember<FaceControllers>);
}
