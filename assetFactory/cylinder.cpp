#include "cylinder.h"
#include "maths.h"
#include "modelFactoryMesh.h"
#include <ranges>

Cylinder::CreatedFaces
Cylinder::createMesh(ModelFactoryMesh & mesh, Scale3D lodf) const
{
	const auto P = std::max(5U, static_cast<unsigned int>(std::round(10.F * std::max(lodf.x, lodf.y))));
	const auto step = two_pi / static_cast<float>(P);

	// Generate 2D circumference points
	std::vector<RelativePosition2D> circumference(P);
	std::generate(circumference.begin(), circumference.end(), [a = 0.F, step]() mutable {
		return sincos(a += step) * .5F;
	});

	CreatedFaces surface;
	{
		// Generate bottom face vertices
		std::vector<OpenMesh::VertexHandle> bottom(P);
		std::transform(circumference.begin(), circumference.end(), bottom.begin(), [&mesh](const auto & xy) {
			return mesh.add_vertex(xy || 0.F);
		});
		surface.insert(mesh.add_namedFace("bottom", bottom));
	}
	{
		// Generate top face vertices
		std::vector<OpenMesh::VertexHandle> top(P);
		std::transform(circumference.rbegin(), circumference.rend(), top.begin(), [&mesh](const auto & xy) {
			return mesh.add_vertex(xy || 1.F);
		});
		surface.insert(mesh.add_namedFace("top", top));
	}
	{
		// Generate edge vertices
		std::vector<std::pair<OpenMesh::VertexHandle, OpenMesh::VertexHandle>> edge(P + 1);
		std::transform(circumference.begin(), circumference.end(), edge.begin(), [&mesh](const auto & xy) {
			return std::make_pair(mesh.add_vertex(xy || 0.F), mesh.add_vertex(xy || 1.F));
		});
		// Wrap around
		edge.back() = edge.front();
		// Transform adjacent pairs of top/bottom pairs to faces
		for (const auto & [first, second] : edge | std::views::adjacent<2>) {
			const auto fh
					= surface.insert(mesh.add_namedFace("edge", first.first, first.second, second.second, second.first))
							  ->second;
			mesh.property(mesh.smoothFaceProperty, fh) = true;
		}
	}

	return surface;
}
