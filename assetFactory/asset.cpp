#include "asset.h"
#include "assetFactory.h"

bool
Asset::persist(Persistence::PersistenceStore & store)
{
	return STORE_MEMBER(id) && STORE_MEMBER(name);
}

std::any
Asset::createAt(const Location &) const
{
	return {};
}

Asset::TexturePtr
Asset::getTexture() const
{
	if (auto mf = Persistence::ParseBase::getShared<const AssetFactory>("assetFactory")) {
		return mf->getTexture();
	}
	return nullptr;
}

Asset::MeshConstruct::MeshConstruct(Mesh::Ptr & m) :
	Persistence::SelectionPtrBase<FactoryMesh::Ptr> {fmesh}, out {m} { }

void
Asset::MeshConstruct::endObject(Persistence::Stack & stk)
{
	out = fmesh->createMesh();
	Persistence::SelectionPtrBase<FactoryMesh::Ptr>::endObject(stk);
}

Asset::MeshArrayConstruct::MeshArrayConstruct(std::span<Mesh::Ptr> m) :
	Persistence::SelectionPtrBase<FactoryMesh::Ptr> {fmesh}, out {m}
{
}

void
Asset::MeshArrayConstruct::endObject(Persistence::Stack & stk)
{
	for (auto & outMesh : out) {
		if (!outMesh) {
			outMesh = fmesh->createMesh();
			break;
		}
	}
	Persistence::SelectionPtrBase<FactoryMesh::Ptr>::endObject(stk);
}
