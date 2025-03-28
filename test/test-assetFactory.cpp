#define BOOST_TEST_MODULE test_asset_factory

#include "testHelpers.h"
#include "testRenderOutput.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include "assetFactory/assetFactory.h"
#include "assetFactory/object.h"
#include "assetFactory/texturePacker.h"
#include "game/scenary/foliage.h"
#include "game/scenary/illuminator.h"
#include "game/scenary/light.h"
#include "game/scenary/plant.h"
#include "game/surface.h"
#include "game/vehicles/railVehicle.h"
#include "game/vehicles/railVehicleClass.h"
#include "gfx/gl/sceneRenderer.h"
#include "gfx/renderable.h"
#include "lib/collection.h"
#include "lib/location.h"
#include "lib/stream_support.h"
#include "testMainWindow.h"

BOOST_GLOBAL_FIXTURE(TestMainWindowAppBase);

const std::filesystem::path TMP {"/tmp"};

class FactoryFixture : public TestRenderOutputSize<TextureAbsCoord {2048, 1024}>, public SceneProvider {
public:
	FactoryFixture() : sceneRenderer {size, output} { }

	~FactoryFixture()
	{
		auto outpath = (TMP / boost::unit_test::framework::current_test_case().full_name()).replace_extension(".tga");
		std::filesystem::create_directories(outpath.parent_path());
		Texture::save(outImage, outpath.c_str());
	}

	void
	content(const SceneShader & shader, const Frustum & frustum) const override
	{
		shader.basic.use(Location {{0, 0, 0}, {0, 0, 0}});
		objects.apply(&Renderable::render, shader, frustum);
	}

	void
	lights(const SceneShader & shader) const override
	{
		objects.apply(&Renderable::lights, shader);
	}

	void
	environment(const SceneShader &, const SceneRenderer & sceneRenderer) const override
	{
		sceneRenderer.setAmbientLight({.4, .4, .4});
		sceneRenderer.setDirectionalLight({.6, .6, .6}, {{0.9, 0.5}}, *this);
	}

	void
	shadows(const ShadowMapper & mapper, const Frustum & frustum) const override
	{
		mapper.dynamicPoint.use(Location {{0, 0, 0}, {0, 0, 0}});
		objects.apply(&Renderable::shadows, mapper, frustum);
	}

	void
	render(float dist)
	{
		sceneRenderer.camera.setView({-dist, dist * 1.2f, dist * 1.2f}, south + east + down);
		sceneRenderer.render(*this);
	}

	SharedCollection<const Renderable> objects;

private:
	SceneRenderer sceneRenderer;
};

BOOST_AUTO_TEST_CASE(surfaces, *boost::unit_test::timeout(5))
{
	auto mf = AssetFactory::loadXML(RESDIR "/surfaces.xml");
	BOOST_REQUIRE(mf);
	BOOST_CHECK_EQUAL(4, mf->assets.size());
	auto gravelAsset = mf->assets.at("terrain.surface.gravel");
	BOOST_REQUIRE(gravelAsset);
	auto gravel = gravelAsset.dynamicCast<Surface>();
	BOOST_REQUIRE(gravel);
	BOOST_REQUIRE_EQUAL(gravel->name, "Gravel");
	BOOST_REQUIRE_EQUAL(gravel->colorBias, RGB {.9F});
	BOOST_REQUIRE_EQUAL(gravel->quality, 1.F);
}

BOOST_FIXTURE_TEST_SUITE(m, FactoryFixture);

BOOST_AUTO_TEST_CASE(brush47xml, *boost::unit_test::timeout(5))
{
	auto mf = AssetFactory::loadXML(RESDIR "/brush47.xml");
	BOOST_REQUIRE(mf);
	gameState.assets = mf->assets;
	BOOST_REQUIRE_GE(mf->shapes.size(), 6);
	BOOST_CHECK(mf->shapes.at("plane"));
	BOOST_CHECK(mf->shapes.at("cylinder"));
	BOOST_CHECK(mf->shapes.at("cuboid"));
	BOOST_CHECK(mf->shapes.at("wheel"));
	BOOST_CHECK(mf->shapes.at("axel"));
	auto bogie = mf->shapes.at("bogie");
	BOOST_REQUIRE(bogie);
	auto bogieObj = std::dynamic_pointer_cast<const Object>(bogie);
	BOOST_CHECK_GE(bogieObj->uses.size(), 3);
	BOOST_CHECK_EQUAL(1, mf->assets.size());
	auto brush47 = mf->assets.at("brush-47");
	BOOST_REQUIRE(brush47);
	auto brush47rvc = brush47.dynamicCast<RailVehicleClass>();
	BOOST_REQUIRE(brush47rvc);
	BOOST_REQUIRE(brush47rvc->bodyMesh);
	BOOST_REQUIRE(brush47rvc->bogies.front());
	BOOST_REQUIRE(brush47rvc->bogies.back());

	auto railVehicle = std::make_shared<RailVehicle>(brush47rvc);
	objects.emplace(brush47rvc);

	render(10000);
}

BOOST_AUTO_TEST_CASE(foliage, *boost::unit_test::timeout(5))
{
	auto mf = AssetFactory::loadXML(RESDIR "/foliage.xml");
	BOOST_REQUIRE(mf);
	gameState.assets = mf->assets;
	auto tree_01_1 = mf->assets.at("Tree-01-1");
	BOOST_REQUIRE(tree_01_1);
	auto tree_01_1_f = tree_01_1.dynamicCast<Foliage>();
	BOOST_REQUIRE(tree_01_1_f);

	auto plant1 = std::make_shared<Plant>(tree_01_1_f, Location {{-2000, 2000, 0}, {0, 0, 0}});
	auto plant2 = std::make_shared<Plant>(tree_01_1_f, Location {{3000, -4000, 0}, {0, 1, 0}});
	auto plant3 = std::make_shared<Plant>(tree_01_1_f, Location {{-2000, -4000, 0}, {0, 2, 0}});
	auto plant4 = std::make_shared<Plant>(tree_01_1_f, Location {{3000, 2000, 0}, {0, 3, 0}});
	objects.emplace(tree_01_1_f);

	render(6000);
}

BOOST_AUTO_TEST_CASE(lights, *boost::unit_test::timeout(5))
{
	auto mf = AssetFactory::loadXML(RESDIR "/lights.xml");
	BOOST_REQUIRE(mf);
	gameState.assets = mf->assets;
	auto rlight = mf->assets.at("r-light");
	BOOST_REQUIRE(rlight);
	auto oldlamp = mf->assets.at("old-lamp");
	BOOST_REQUIRE(oldlamp);
	auto rlight_f = rlight.dynamicCast<Illuminator>();
	BOOST_REQUIRE(rlight_f);
	auto oldlamp_f = oldlamp.dynamicCast<Illuminator>();
	BOOST_REQUIRE(oldlamp_f);

	auto light1 = std::make_shared<Light>(oldlamp_f, Location {{0, 0, 0}, {0, 0, 0}});
	auto light2 = std::make_shared<Light>(rlight_f, Location {{-4000, 0, 0}, {0, 2, 0}});
	auto light3 = std::make_shared<Light>(rlight_f, Location {{-4000, -4000, 0}, {0, 1, 0}});
	auto light4 = std::make_shared<Light>(oldlamp_f, Location {{3000, 4600, 0}, {0, 2, 0}});
	objects.emplace(rlight_f);
	objects.emplace(oldlamp_f);

	// yes I'm hacking some floor to light up as though its a bush
	auto floorf = mf->assets.at("floor").dynamicCast<Foliage>();
	auto floor = std::make_shared<Plant>(floorf, Location {});
	objects.emplace(floorf);

	render(6000);
}

BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_CASE(loadall)
{
	const auto assets = AssetFactory::loadAll(RESDIR);
	BOOST_CHECK(assets.at("brush-47"));
	BOOST_CHECK(assets.at("Tree-01-1"));
}

template<typename T> using InOut = std::tuple<T, T>;

BOOST_DATA_TEST_CASE(normalizeColourName,
		boost::unit_test::data::make<InOut<std::string>>({
				{"", ""},
				{"black", "black"},
				{" black ", "black"},
				{" b l a c k ", "black"},
				{" B L A c k ", "black"},
				{"BLAck ", "black"},
				{"BLACK ", "black"},
				{"BlAck ", "black"},
				{"Bl Ack ", "black"},
		}),
		in_, exp)
{
	auto in {in_};
	BOOST_CHECK_NO_THROW(AssetFactory::normalizeColourName(in));
	BOOST_CHECK_EQUAL(in, exp);
}

BOOST_AUTO_TEST_CASE(parseX11RGB, *boost::unit_test::timeout(5))
{
	const auto parsedColours = AssetFactory::parseX11RGB(FIXTURESDIR "rgb.txt");
	BOOST_REQUIRE_EQUAL(parsedColours.size(), 20);
	BOOST_CHECK_CLOSE_VEC(parsedColours.at("cyan"), AssetFactory::Colour(0, 1, 1));
	BOOST_CHECK_CLOSE_VEC(parsedColours.at("slategrey"), AssetFactory::Colour(0.44F, 0.5, 0.56F));
	BOOST_CHECK_CLOSE_VEC(parsedColours.at("lightsteelblue1"), AssetFactory::Colour(0.79, 0.88, 1));
}

BOOST_AUTO_TEST_CASE(texturePacker, *boost::unit_test::timeout(5))
{
	std::vector<TexturePacker::Image> input {
			{10, 10},
			{10, 10},
			{10, 10},
			{100, 10},
			{10, 200},
			{5, 5},
	};
	TexturePacker tp {input};
	BOOST_CHECK_EQUAL(TexturePacker::Size(128, 256), tp.minSize());
	const auto result = tp.pack();
}

BOOST_AUTO_TEST_CASE(texturePacker_many, *boost::unit_test::timeout(5))
{
	std::vector<TexturePacker::Image> images(256);
	std::fill(images.begin(), images.end(), TexturePacker::Image {32, 32});
	const auto totalSize = std::accumulate(images.begin(), images.end(), 0, [](auto t, const auto & i) {
		return t + TexturePacker::area(i);
	});
	TexturePacker tp {images};
	BOOST_CHECK_EQUAL(TexturePacker::Size(32, 32), tp.minSize());
	const auto result = tp.pack();
	BOOST_CHECK_EQUAL(result.first.size(), images.size());
	BOOST_CHECK_GE(TexturePacker::area(result.second),
			TexturePacker::area(images.front()) * static_cast<GLsizei>(images.size()));
	BOOST_CHECK_EQUAL(totalSize, TexturePacker::area(result.second));
}

BOOST_AUTO_TEST_CASE(texturePacker_many_random, *boost::unit_test::timeout(15))
{
	std::vector<TexturePacker::Image> images(2048);
	std::mt19937 gen(std::random_device {}());
	std::uniform_int_distribution<> dim {1, 10};
	std::generate(images.begin(), images.end(), [&dim, &gen]() {
		return TexturePacker::Image {2 ^ dim(gen), 2 ^ dim(gen)};
	});
	TexturePacker tp {images};
	const auto result = tp.pack();
	BOOST_CHECK_EQUAL(result.first.size(), images.size());
}
