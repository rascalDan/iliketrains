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
#include "lib/stream_support.h" // IWYU pragma: keep
#include "testMainWindow.h"
#include <special_members.h>

BOOST_GLOBAL_FIXTURE(TestMainWindowAppBase);

namespace {
	class FactoryFixture : public TestRenderOutputSize<TextureAbsCoord {2048, 1024}>, public SceneProvider {
	public:
		FactoryFixture() : sceneRenderer {size, output} { }

		NO_COPY(FactoryFixture);
		NO_MOVE(FactoryFixture);

		~FactoryFixture() override
		{
			auto outpath = (ANALYSIS_DIRECTORY / boost::unit_test::framework::current_test_case().full_name())
								   .replace_extension(".tga");
			std::filesystem::create_directories(outpath.parent_path());
			Texture::save(outImage, outpath.c_str());
		}

		void
		content(const SceneShader & shader, const Frustum & frustum) const override
		{
			shader.basic.use(Location {.pos = {0, 0, 0}, .rot = {0, 0, 0}});
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
			mapper.dynamicPoint.use(Location {.pos = {0, 0, 0}, .rot = {0, 0, 0}});
			objects.apply(&Renderable::shadows, mapper, frustum);
		}

		void
		render(float dist)
		{
			sceneRenderer.camera.setView({-dist, dist * 1.2F, dist * 1.2F}, south + east + down);
			sceneRenderer.render(*this);
		}

		SharedCollection<const Renderable> objects;

	private:
		SceneRenderer sceneRenderer;
	};
}

BOOST_AUTO_TEST_CASE(Surfaces, *boost::unit_test::timeout(5))
{
	auto factory = AssetFactory::loadXML(RESDIR "/surfaces.xml");
	BOOST_REQUIRE(factory);
	BOOST_CHECK_EQUAL(4, factory->assets.size());
	auto gravelAsset = factory->assets.at("terrain.surface.gravel");
	BOOST_REQUIRE(gravelAsset);
	auto gravel = gravelAsset.dynamicCast<Surface>();
	BOOST_REQUIRE(gravel);
	BOOST_REQUIRE_EQUAL(gravel->name, "Gravel");
	BOOST_REQUIRE_EQUAL(gravel->colorBias, RGB {.9F});
	BOOST_REQUIRE_EQUAL(gravel->quality, 1.F);
}

BOOST_FIXTURE_TEST_SUITE(m, FactoryFixture);

BOOST_AUTO_TEST_CASE(Brush47xml, *boost::unit_test::timeout(5))
{
	auto factory = AssetFactory::loadXML(RESDIR "/brush47.xml");
	BOOST_REQUIRE(factory);
	gameState.assets = factory->assets;
	BOOST_REQUIRE_GE(factory->shapes.size(), 6);
	BOOST_CHECK(factory->shapes.at("plane"));
	BOOST_CHECK(factory->shapes.at("cylinder"));
	BOOST_CHECK(factory->shapes.at("cuboid"));
	BOOST_CHECK(factory->shapes.at("wheel"));
	BOOST_CHECK(factory->shapes.at("axel"));
	auto bogie = factory->shapes.at("bogie");
	BOOST_REQUIRE(bogie);
	auto bogieObj = std::dynamic_pointer_cast<const Object>(bogie);
	BOOST_CHECK_GE(bogieObj->uses.size(), 3);
	BOOST_CHECK_EQUAL(1, factory->assets.size());
	auto brush47 = factory->assets.at("brush-47");
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

BOOST_AUTO_TEST_CASE(Tree, *boost::unit_test::timeout(5))
{
	auto factory = AssetFactory::loadXML(RESDIR "/foliage.xml");
	BOOST_REQUIRE(factory);
	gameState.assets = factory->assets;
	auto tree011Asset = factory->assets.at("Tree-01-1");
	BOOST_REQUIRE(tree011Asset);
	auto tree011 = tree011Asset.dynamicCast<Foliage>();
	BOOST_REQUIRE(tree011);

	auto plant1 = std::make_shared<Plant>(tree011, Location {.pos = {-2000, 2000, 0}, .rot = {0, 0, 0}});
	auto plant2 = std::make_shared<Plant>(tree011, Location {.pos = {3000, -4000, 0}, .rot = {0, 1, 0}});
	auto plant3 = std::make_shared<Plant>(tree011, Location {.pos = {-2000, -4000, 0}, .rot = {0, 2, 0}});
	auto plant4 = std::make_shared<Plant>(tree011, Location {.pos = {3000, 2000, 0}, .rot = {0, 3, 0}});
	objects.emplace(tree011);

	render(6000);
}

BOOST_AUTO_TEST_CASE(Lights, *boost::unit_test::timeout(5))
{
	auto factory = AssetFactory::loadXML(RESDIR "/lights.xml");
	BOOST_REQUIRE(factory);
	gameState.assets = factory->assets;
	auto rlightAsset = factory->assets.at("r-light");
	BOOST_REQUIRE(rlightAsset);
	auto oldlampAsset = factory->assets.at("old-lamp");
	BOOST_REQUIRE(oldlampAsset);
	auto rlight = rlightAsset.dynamicCast<Illuminator>();
	BOOST_REQUIRE(rlight);
	auto oldlamp = oldlampAsset.dynamicCast<Illuminator>();
	BOOST_REQUIRE(oldlamp);

	auto light1 = std::make_shared<Light>(oldlamp, Location {.pos = {0, 0, 0}, .rot = {0, 0, 0}});
	auto light2 = std::make_shared<Light>(rlight, Location {.pos = {-4000, 0, 0}, .rot = {0, 2, 0}});
	auto light3 = std::make_shared<Light>(rlight, Location {.pos = {-4000, -4000, 0}, .rot = {0, 1, 0}});
	auto light4 = std::make_shared<Light>(oldlamp, Location {.pos = {3000, 4600, 0}, .rot = {0, 2, 0}});
	objects.emplace(rlight);
	objects.emplace(oldlamp);

	// yes I'm hacking some floor to light up as though its a bush
	auto floorf = factory->assets.at("floor").dynamicCast<Foliage>();
	auto floor = std::make_shared<Plant>(floorf, Location {});
	objects.emplace(floorf);

	render(6000);
}

BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_CASE(Loadall)
{
	const auto assets = AssetFactory::loadAll(RESDIR);
	BOOST_CHECK(assets.at("brush-47"));
	BOOST_CHECK(assets.at("Tree-01-1"));
}

template<typename T> using InOut = std::tuple<T, T>;

BOOST_DATA_TEST_CASE(NormalizeColourName,
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
		input, exp)
{
	auto toNormalize {input};
	BOOST_CHECK_NO_THROW(AssetFactory::normalizeColourName(toNormalize));
	BOOST_CHECK_EQUAL(toNormalize, exp);
}

BOOST_AUTO_TEST_CASE(ParseX11RGB, *boost::unit_test::timeout(5))
{
	const auto parsedColours = AssetFactory::parseX11RGB(FIXTURESDIR "rgb.txt");
	BOOST_REQUIRE_EQUAL(parsedColours.size(), 20);
	BOOST_CHECK_CLOSE_VEC(parsedColours.at("cyan"), AssetFactory::Colour(0, 1, 1));
	BOOST_CHECK_CLOSE_VEC(parsedColours.at("slategrey"), AssetFactory::Colour(0.44F, 0.5, 0.56F));
	BOOST_CHECK_CLOSE_VEC(parsedColours.at("lightsteelblue1"), AssetFactory::Colour(0.79, 0.88, 1));
}

BOOST_AUTO_TEST_CASE(TexturePackerSimple, *boost::unit_test::timeout(5))
{
	std::vector<TexturePacker::Image> input {
			{10, 10},
			{10, 10},
			{10, 10},
			{100, 10},
			{10, 200},
			{5, 5},
	};
	TexturePacker packer {input};
	BOOST_CHECK_EQUAL(TexturePacker::Size(128, 256), packer.minSize());
	const auto result = packer.pack();
}

BOOST_AUTO_TEST_CASE(TexturePackerMany, *boost::unit_test::timeout(5))
{
	std::vector<TexturePacker::Image> images(256);
	std::fill(images.begin(), images.end(), TexturePacker::Image {32, 32});
	const auto totalSize = std::ranges::fold_left(images, 0, [](auto total, const auto & image) {
		return total + TexturePacker::area(image);
	});
	TexturePacker packer {images};
	BOOST_CHECK_EQUAL(TexturePacker::Size(32, 32), packer.minSize());
	const auto result = packer.pack();
	BOOST_CHECK_EQUAL(result.first.size(), images.size());
	BOOST_CHECK_GE(TexturePacker::area(result.second),
			TexturePacker::area(images.front()) * static_cast<GLsizei>(images.size()));
	BOOST_CHECK_EQUAL(totalSize, TexturePacker::area(result.second));
}

BOOST_AUTO_TEST_CASE(TexturePackerManyRandom, *boost::unit_test::timeout(15))
{
	std::vector<TexturePacker::Image> images(2048);
	std::mt19937 gen(std::random_device {}());
	std::uniform_int_distribution<> dim {1, 10};
	std::generate(images.begin(), images.end(), [&dim, &gen]() {
		return TexturePacker::Image {2 ^ dim(gen), 2 ^ dim(gen)};
	});
	TexturePacker packer {images};
	const auto result = packer.pack();
	BOOST_CHECK_EQUAL(result.first.size(), images.size());
}
