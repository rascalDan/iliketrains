#include "asset.h"

bool
Asset::persist(Persistence::PersistenceStore & store)
{
	return STORE_MEMBER(id) && STORE_MEMBER(name)
			&& STORE_NAME_HELPER("mesh", meshes, Persistence::Appender<FactoryMesh::Collection>);
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
