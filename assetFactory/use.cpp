#include "use.h"
#include "assetFactory.h"

Shape::CreatedFaces
Use::createMesh(ModelFactoryMesh & mesh, const Mutation::Matrix & mutation) const
{
	auto faces = type->createMesh(mesh, mutation * getMatrix());
	applyStyle(mesh, {this}, faces);
	for (const auto & [name, faceController] : faceControllers) {
		faceController->apply(mesh, {this}, name, faces);
	}
	return faces;
}

struct Lookup : public Persistence::SelectionV<Shape::CPtr> {
	using Persistence::SelectionV<Shape::CPtr>::SelectionV;
	using Persistence::SelectionV<Shape::CPtr>::setValue;
	void
	setValue(std::string && str) override
	{
		if (auto mf = std::dynamic_pointer_cast<const AssetFactory>(Persistence::sharedObjects.at("assetFactory"))) {
			v = mf->shapes.at(str);
		}
	}
};

bool
Use::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_HELPER(type, Lookup) && STORE_MEMBER(position) && STORE_MEMBER(scale)
			&& STORE_MEMBER(rotation) && Style::persist(store)
			&& STORE_NAME_HELPER("face", faceControllers, Persistence::MapByMember<FaceControllers>);
}
