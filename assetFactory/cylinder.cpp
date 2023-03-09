#include "cylinder.h"
#include "maths.h"
#include "modelFactoryMesh.h"
#include <stream_support.hpp>

Cylinder::CreatedFaces
Cylinder::createMesh(ModelFactoryMesh & mesh, float lodf) const
{
	const unsigned int P = static_cast<unsigned int>(std::round(15.F * std::sqrt(lodf)));
	std::vector<OpenMesh::VertexHandle> bottom(P), top(P);
	std::generate_n(bottom.begin(), P, [a = 0.f, step = two_pi / static_cast<float>(P), &mesh]() mutable {
		const auto xy = sincosf(a += step) * .5F;
		return mesh.add_vertex({xy.x, xy.y, 0.f});
	});
	std::generate_n(top.begin(), P, [a = 0.f, step = two_pi / static_cast<float>(P), &mesh]() mutable {
		const auto xy = sincosf(a -= step) * .5F;
		return mesh.add_vertex({xy.x, xy.y, 1.f});
	});
	CreatedFaces surface;
	std::generate_n(std::inserter(surface, surface.end()), P,
			[a = 0.f, step = two_pi / static_cast<float>(P), &mesh]() mutable {
				const auto xy1 = sincosf(a) * .5F;
				const auto xy2 = sincosf(a -= step) * .5F;
				const auto xyz1b = (xy1 ^ 0);
				const auto xyz2b = (xy2 ^ 0);
				const auto xyz1t = (xy1 ^ 1);
				const auto xyz2t = (xy2 ^ 1);
				return mesh.add_namedFace("edge",
						{
								mesh.add_vertex({xyz1b.x, xyz1b.y, xyz1b.z}),
								mesh.add_vertex({xyz2b.x, xyz2b.y, xyz2b.z}),
								mesh.add_vertex({xyz2t.x, xyz2t.y, xyz2t.z}),
								mesh.add_vertex({xyz1t.x, xyz1t.y, xyz1t.z}),
						});
			});
	for (const auto & [name, face] : surface) {
		mesh.property(mesh.smoothFaceProperty, face) = true;
	}
	surface.insert(mesh.add_namedFace("bottom", bottom));
	surface.insert(mesh.add_namedFace("top", top));

	return surface;
}
