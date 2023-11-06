#define BOOST_TEST_MODULE test_asset_factory

#include "testHelpers.h"
#include "testRenderOutput.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include "assetFactory/assetFactory.h"
#include "assetFactory/object.h"
#include "assetFactory/texturePacker.h"
#include "game/scenary/foliage.h"
#include "game/scenary/plant.h"
#include "game/vehicles/railVehicle.h"
#include "game/vehicles/railVehicleClass.h"
#include "gfx/gl/sceneRenderer.h"
#include "gfx/renderable.h"
#include "lib/collection.h"
#include "lib/location.h"
#include "lib/stream_support.h"
#include "testMainWindow.h"
#include "ui/applicationBase.h"

BOOST_GLOBAL_FIXTURE(ApplicationBase);
BOOST_GLOBAL_FIXTURE(TestMainWindow);

const std::filesystem::path TMP {"/tmp"};

class FactoryFixture : public TestRenderOutputSize<glm::ivec2 {2048, 1024}>, public SceneProvider {
public:
	FactoryFixture() : sceneRenderer {size, output} { }

	~FactoryFixture()
	{
		auto outpath = (TMP / boost::unit_test::framework::current_test_case().full_name()).replace_extension(".tga");
		std::filesystem::create_directories(outpath.parent_path());
		Texture::save(outImage, outpath.c_str());
	}

	void
	content(const SceneShader & shader) const override
	{
		shader.basic.use(Location {{0, 0, 0}, {0, 0, 0}});
		objects.apply(&Renderable::render, shader);
	}

	void
	lights(const SceneShader & shader) const override
	{
		shader.pointLight.add({-3, 1, 5}, {1, 1, 1}, .1F);
	}

	void
	environment(const SceneShader &, const SceneRenderer & sceneRenderer) const override
	{
		sceneRenderer.setAmbientLight({.4, .4, .4});
		sceneRenderer.setDirectionalLight({.6, .6, .6}, east + south + south + down, *this);
	}

	void
	shadows(const ShadowMapper & mapper) const override
	{
		mapper.dynamicPoint.use(Location {{0, 0, 0}, {0, 0, 0}});
		objects.apply(&Renderable::shadows, mapper);
	}

	void
	render(float dist = 10.f)
	{
		sceneRenderer.camera.setView({-dist, dist * 1.2f, dist * 1.2f}, south + east + down);
		sceneRenderer.render(*this);
	}

	Collection<const Renderable> objects;

private:
	SceneRenderer sceneRenderer;
};

BOOST_FIXTURE_TEST_SUITE(m, FactoryFixture);

BOOST_AUTO_TEST_CASE(brush47xml, *boost::unit_test::timeout(5))
{
	auto mf = AssetFactory::loadXML(RESDIR "/brush47.xml");
	BOOST_REQUIRE(mf);
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
	auto brush47rvc = std::dynamic_pointer_cast<RailVehicleClass>(brush47);
	BOOST_REQUIRE(brush47rvc);
	BOOST_REQUIRE(brush47rvc->bodyMesh);
	BOOST_REQUIRE(brush47rvc->bogies.front());
	BOOST_REQUIRE(brush47rvc->bogies.back());

	auto railVehicle = std::make_shared<RailVehicle>(brush47rvc);
	objects.objects.push_back(brush47rvc);

	render();
}

BOOST_AUTO_TEST_CASE(foliage, *boost::unit_test::timeout(5))
{
	auto mf = AssetFactory::loadXML(RESDIR "/foliage.xml");
	BOOST_REQUIRE(mf);
	auto tree_01_1 = mf->assets.at("Tree-01-1");
	BOOST_REQUIRE(tree_01_1);
	auto tree_01_1_f = std::dynamic_pointer_cast<Foliage>(tree_01_1);
	BOOST_REQUIRE(tree_01_1_f);

	auto plant1 = std::make_shared<Plant>(tree_01_1_f, Location {{-2, 2, 0}, {0, 0, 0}});
	auto plant2 = std::make_shared<Plant>(tree_01_1_f, Location {{3, -4, 0}, {0, 1, 0}});
	auto plant3 = std::make_shared<Plant>(tree_01_1_f, Location {{-2, -4, 0}, {0, 2, 0}});
	auto plant4 = std::make_shared<Plant>(tree_01_1_f, Location {{3, 2, 0}, {0, 3, 0}});
	objects.objects.push_back(tree_01_1_f);

	render(5);
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
	const auto totalSize = std::accumulate(images.begin(), images.end(), 0U, [](auto t, const auto & i) {
		return t + TexturePacker::area(i);
	});
	TexturePacker tp {images};
	BOOST_CHECK_EQUAL(TexturePacker::Size(32, 32), tp.minSize());
	const auto result = tp.pack();
	BOOST_CHECK_EQUAL(result.first.size(), images.size());
	BOOST_CHECK_GE(TexturePacker::area(result.second), TexturePacker::area(images.front()) * images.size());
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
