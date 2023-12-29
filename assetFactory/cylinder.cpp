#include "cylinder.h"
#include "maths.h"
#include "modelFactoryMesh.h"

Cylinder::CreatedFaces
Cylinder::createMesh(ModelFactoryMesh & mesh, float lodf) const
{
	const auto P = static_cast<unsigned int>(std::round(15.F * std::sqrt(lodf)));
	const auto step = two_pi / static_cast<float>(P);

	// Generate 2D circumference points
	std::vector<Position2D> circumference(P);
	std::generate(circumference.begin(), circumference.end(), [a = 0.F, step]() mutable {
		return sincosf(a += step) * .5F;
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
		std::adjacent_find(edge.begin(), edge.end(), [&mesh, &surface](const auto & first, const auto & second) {
			const auto fh
					= surface.insert(mesh.add_namedFace("edge", first.first, first.second, second.second, second.first))
							  ->second;
			mesh.property(mesh.smoothFaceProperty, fh) = true;
			return false;
		});
	}

	return surface;
}
