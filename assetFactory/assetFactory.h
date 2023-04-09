#pragma once

#include "asset.h"
#include "persistence.h"
#include "shape.h"
#include "textureFragment.h"
#include <filesystem>

class Texture;

class AssetFactory : public Persistence::Persistable {
public:
	using Shapes = std::map<std::string, Shape::Ptr, std::less<>>;
	using Assets = std::map<std::string, Asset::Ptr, std::less<>>;
	using TextureFragments = std::map<std::string, TextureFragment::Ptr, std::less<>>;
	using Colour = glm::vec3;
	using ColourAlpha = glm::vec4;
	using Colours = std::map<std::string, Colour, std::less<>>;
	using TextureFragmentCoords = std::array<glm::vec2, 4>;

	AssetFactory();
	[[nodiscard]] static std::shared_ptr<AssetFactory> loadXML(const std::filesystem::path &);
	[[nodiscard]] ColourAlpha parseColour(std::string_view) const;
	[[nodiscard]] TextureFragmentCoords getTextureCoords(std::string_view) const;
	[[nodiscard]] Asset::TexturePtr getTexture() const;

	Shapes shapes;
	Assets assets;
	Colours colours;
	TextureFragments textureFragments;

	static Colours parseX11RGB(const char * rgbtxtpath);
	static void normalizeColourName(std::string &);

private:
	friend Persistence::SelectionPtrBase<std::shared_ptr<AssetFactory>>;
	bool persist(Persistence::PersistenceStore & store) override;

	void createTexutre() const;

	mutable Asset::TexturePtr texture;
	mutable std::map<std::string_view, glm::vec4, std::less<>> textureFragmentPositions;
};
