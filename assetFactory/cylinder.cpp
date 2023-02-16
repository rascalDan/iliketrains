#include "cylinder.h"
#include "maths.h"
#include "modelFactoryMesh.h"

Cylinder::CreatedFaces
Cylinder::createMesh(ModelFactoryMesh & mesh, const Mutation::Matrix & mutation) const
{
	const glm::vec2 scale {std::accumulate(&mutation[0][0], &mutation[0][3], 0.f),
			std::accumulate(&mutation[1][0], &mutation[1][3], 0.f)};
	const unsigned int P = static_cast<unsigned int>(std::round(15.F * std::sqrt(glm::length(scale))));
	std::vector<OpenMesh::VertexHandle> bottom(P), top(P);
	std::generate_n(bottom.begin(), P, [a = 0.f, step = two_pi / static_cast<float>(P), &mesh, &mutation]() mutable {
		const auto xy = sincosf(a += step) * .5F;
		const auto xyz = (xy ^ 0) % mutation;
		return mesh.add_vertex({xyz.x, xyz.y, xyz.z});
	});
	std::generate_n(top.begin(), P, [a = 0.f, step = two_pi / static_cast<float>(P), &mesh, &mutation]() mutable {
		const auto xy = sincosf(a -= step) * .5F;
		const auto xyz = (xy ^ 1) % mutation;
		return mesh.add_vertex({xyz.x, xyz.y, xyz.z});
	});
	CreatedFaces surface;
	std::generate_n(std::inserter(surface, surface.end()), P,
			[a = 0.f, step = two_pi / static_cast<float>(P), &mesh, &mutation]() mutable {
				const auto xy1 = sincosf(a) * .5F;
				const auto xy2 = sincosf(a -= step) * .5F;
				const auto xyz1b = (xy1 ^ 0) % mutation;
				const auto xyz2b = (xy2 ^ 0) % mutation;
				const auto xyz1t = (xy1 ^ 1) % mutation;
				const auto xyz2t = (xy2 ^ 1) % mutation;
				return std::make_pair(std::string {"edge"},
						mesh.add_face({
								mesh.add_vertex({xyz1b.x, xyz1b.y, xyz1b.z}),
								mesh.add_vertex({xyz2b.x, xyz2b.y, xyz2b.z}),
								mesh.add_vertex({xyz2t.x, xyz2t.y, xyz2t.z}),
								mesh.add_vertex({xyz1t.x, xyz1t.y, xyz1t.z}),
						}));
			});
	surface.emplace("bottom", mesh.add_face(bottom));
	surface.emplace("top", mesh.add_face(top));

	return surface;
}
