#include "object.h"
#include <algorithm>
#include <format>

Object::Object(std::string i) : id {std::move(i)} { }

Object::CreatedFaces
Object::createMesh(ModelFactoryMesh & mesh, Scale3D levelOfDetailFactor) const
{
	CreatedFaces faces;
	for (const auto & use : uses) {
		auto useFaces = use->createMesh(mesh, levelOfDetailFactor);
		std::transform(useFaces.begin(), useFaces.end(), std::inserter(faces, faces.end()), [this](auto && face) {
			return std::make_pair(std::format("{}:{}", id, face.first), std::move(face.second));
		});
	}
	return faces;
}

bool
Object::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_MEMBER(id) && STORE_NAME_HELPER("use", uses, Persistence::Appender<Use::Collection>);
}
