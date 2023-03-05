#pragma once

#include "asset.h"
#include "persistence.h"
#include "shape.h"
#include <filesystem>

class AssetFactory : public Persistence::Persistable {
public:
	using Shapes = std::map<std::string, Shape::Ptr, std::less<>>;
	using Assets = std::map<std::string, Asset::Ptr, std::less<>>;
	using Colour = glm::vec3;
	using ColourAlpha = glm::vec4;
	using Colours = std::map<std::string, Colour, std::less<>>;

	AssetFactory();
	[[nodiscard]] static std::shared_ptr<AssetFactory> loadXML(const std::filesystem::path &);
	[[nodiscard]] ColourAlpha parseColour(std::string_view) const;

	Shapes shapes;
	Assets assets;
	Colours colours;

	static Colours parseX11RGB(const char * rgbtxtpath);
	static void normalizeColourName(std::string &);

private:
	friend Persistence::SelectionPtrBase<std::shared_ptr<AssetFactory>>;
	bool persist(Persistence::PersistenceStore & store) override;
};
