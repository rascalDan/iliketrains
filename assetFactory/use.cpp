#include "use.h"
#include "assetFactory.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/matrix.hpp>
#include <maths.h>
#include <set>
#include <stream_support.hpp>

Shape::CreatedFaces
Use::createMesh(ModelFactoryMesh & mesh, float levelOfDetailFactor) const
{
	auto apply = [&mesh](const auto & faces, const Mutation::Matrix & m) {
		std::set<ModelFactoryMesh::VertexHandle> vs;
		for (const auto & f : faces) {
			const auto fvr = mesh.fv_range(f.second);
			for (const auto & v : fvr) {
				if (!vs.contains(v)) {
					mesh.point(v) %= m;
					vs.insert(v);
				}
			}
		}
	};

	auto faces = type->createMesh(mesh, levelOfDetailFactor * relativeLevelOfDetail());
	applyStyle(mesh, {this}, faces);
	apply(faces, getDeformationMatrix());
	for (const auto & [name, faceController] : faceControllers) {
		faceController->apply(mesh, {this}, name, faces);
	}
	apply(faces, getLocationMatrix());
	return faces;
}

struct Lookup : public Persistence::SelectionV<Shape::CPtr> {
	using Persistence::SelectionV<Shape::CPtr>::SelectionV;
	using Persistence::SelectionV<Shape::CPtr>::setValue;
	void
	setValue(std::string && str) override
	{
		if (auto mf = Persistence::ParseBase::getShared<const AssetFactory>("assetFactory")) {
			v = mf->shapes.at(str);
		}
	}
};

bool
Use::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_HELPER(type, Lookup) && Mutation::persist(store) && Style::persist(store)
			&& STORE_NAME_HELPER("face", faceControllers, Persistence::MapByMember<FaceControllers>);
}
