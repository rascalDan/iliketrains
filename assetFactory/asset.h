#pragma once

#include "factoryMesh.h"
#include "persistence.h"
#include <any>
#include <manyPtr.h>
#include <stdTypeDefs.h>

class TextureAtlas;
class Renderable;
class Location;

class Asset : public Persistence::Persistable, public StdTypeDefs<Asset> {
public:
	using ManyPtr = ManySharedPtr<Asset, const Renderable>;
	using TexturePtr = std::shared_ptr<TextureAtlas>;

	/// Used only for the asset viewer
	[[nodiscard]] virtual std::any createAt(const Location &) const;

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
