#pragma once

#include "factoryMesh.h"
#include "persistence.h"
#include <stdTypeDefs.h>

class TextureAtlas;

class Asset : public Persistence::Persistable, public StdTypeDefs<Asset> {
public:
	using TexturePtr = std::shared_ptr<TextureAtlas>;

	std::string id;
	std::string name;

protected:
	TexturePtr getTexture() const;

	struct MeshConstruct : public Persistence::SelectionPtrBase<FactoryMesh::Ptr> {
		using Persistence::SelectionPtrBase<FactoryMesh::Ptr>::setValue;

		MeshConstruct(Mesh::Ptr & m);

		void endObject(Persistence::Stack & stk) override;

		FactoryMesh::Ptr fmesh;
		Mesh::Ptr & out;
	};

	struct MeshArrayConstruct : public Persistence::SelectionPtrBase<FactoryMesh::Ptr> {
		using Persistence::SelectionPtrBase<FactoryMesh::Ptr>::setValue;

		MeshArrayConstruct(std::span<Mesh::Ptr> m);

		void endObject(Persistence::Stack & stk) override;

		FactoryMesh::Ptr fmesh;
		std::span<Mesh::Ptr> out;
	};

	friend Persistence::SelectionPtrBase<std::shared_ptr<Asset>>;
	bool persist(Persistence::PersistenceStore & store) override;
};
