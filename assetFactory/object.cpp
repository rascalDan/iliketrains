#include "object.h"
#include <algorithm>

Object::Object(std::string i) : id {std::move(i)} { }

Object::CreatedFaces
Object::createMesh(ModelFactoryMesh & mesh, const Mutation::Matrix & mutation) const
{
	CreatedFaces faces;
	for (const auto & use : uses) {
		auto useFaces = use->createMesh(mesh, mutation);
		std::transform(useFaces.begin(), useFaces.end(), std::inserter(faces, faces.end()), [this](auto && face) {
			return std::make_pair(id + ":" + face.first, std::move(face.second));
		});
	}
	return faces;
}
