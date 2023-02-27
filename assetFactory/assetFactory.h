#pragma once

#include "factoryMesh.h"
#include "persistence.h"
#include "shape.h"
#include <filesystem>
#include <stdTypeDefs.hpp>

class Asset : public Persistence::Persistable, public StdTypeDefs<Asset> {
public:
	std::string id;
	std::string name;

	FactoryMesh::Collection meshes;

private:
	friend Persistence::SelectionPtrBase<std::shared_ptr<Asset>, true>;
	bool persist(Persistence::PersistenceStore & store) override;
};

class AssetFactory : public Persistence::Persistable {
public:
	using Shapes = std::map<std::string, Shape::Ptr, std::less<>>;
	using Assets = std::map<std::string, Asset::Ptr, std::less<>>;
	using Colour = glm::u8vec3;
	using Colours = std::map<std::string, Colour, std::less<>>;

	AssetFactory();
	[[nodiscard]] static std::shared_ptr<AssetFactory> loadXML(const std::filesystem::path &);

	Shapes shapes;
	Assets assets;
	Colours colours;

	static Colours parseX11RGB(const char * rgbtxtpath);
	static void normalizeColourName(std::string &);

private:
	friend Persistence::SelectionPtrBase<std::shared_ptr<AssetFactory>, true>;
	bool persist(Persistence::PersistenceStore & store) override;
};
