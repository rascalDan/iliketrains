#define BOOST_TEST_MODULE test_asset_factory

#include "testHelpers.h"
#include "testRenderOutput.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include "assetFactory/assetFactory.h"
#include "assetFactory/object.h"
#include "game/vehicles/railVehicle.h"
#include "game/vehicles/railVehicleClass.h"
#include "gfx/gl/sceneRenderer.h"
#include "gfx/renderable.h"
#include "lib/collection.hpp"
#include "lib/location.hpp"
#include "lib/stream_support.hpp"
#include "testMainWindow.h"
#include "ui/applicationBase.h"

BOOST_GLOBAL_FIXTURE(ApplicationBase);
BOOST_GLOBAL_FIXTURE(TestMainWindow);

const std::filesystem::path TMP {"/tmp"};
class FactoryFixture : public TestRenderOutputSize<glm::ivec2 {2048, 2048}>, public SceneProvider {
public:
	FactoryFixture() : sceneRenderer {size, output} { }
	~FactoryFixture()
	{
		glDisable(GL_DEBUG_OUTPUT);
		auto outpath = (TMP / boost::unit_test::framework::current_test_case().full_name()).replace_extension(".tga");
		std::filesystem::create_directories(outpath.parent_path());
		Texture::save(outImage, size, outpath.c_str());
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
		sceneRenderer.setAmbientLight({.2, .2, .2});
		sceneRenderer.setDirectionalLight({.3, .3, .3}, east + south + south + down, *this);
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
		sceneRenderer.camera.setView({dist, dist, dist}, south + west + down);
		sceneRenderer.render(*this);
	}
	Collection<const Renderable> objects;

private:
	SceneRenderer sceneRenderer;
};

BOOST_FIXTURE_TEST_SUITE(m, FactoryFixture);
BOOST_AUTO_TEST_CASE(brush47xml)
{
	auto mf = AssetFactory::loadXML(RESDIR "/brush47.xml");
	BOOST_REQUIRE(mf);
	BOOST_REQUIRE_EQUAL(6, mf->shapes.size());
	BOOST_CHECK(mf->shapes.at("plane"));
	BOOST_CHECK(mf->shapes.at("cylinder"));
	BOOST_CHECK(mf->shapes.at("cuboid"));
	BOOST_CHECK(mf->shapes.at("wheel"));
	BOOST_CHECK(mf->shapes.at("axel"));
	auto bogie = mf->shapes.at("bogie");
	BOOST_REQUIRE(bogie);
	auto bogieObj = std::dynamic_pointer_cast<const Object>(bogie);
	BOOST_CHECK_EQUAL(3, bogieObj->uses.size());
	BOOST_CHECK_EQUAL(1, mf->assets.size());
	auto brush47 = mf->assets.at("brush-47");
	BOOST_REQUIRE(brush47);
	auto brush47rvc = std::dynamic_pointer_cast<RailVehicleClass>(brush47);
	BOOST_REQUIRE(brush47rvc);
	BOOST_REQUIRE(brush47rvc->bodyMesh);
	BOOST_REQUIRE(brush47rvc->bogies.front());
	BOOST_REQUIRE(brush47rvc->bogies.back());

	auto railVehicle = std::make_shared<RailVehicle>(brush47rvc);
	objects.objects.push_back(railVehicle);

	render(20);
}
BOOST_AUTO_TEST_SUITE_END();

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

BOOST_AUTO_TEST_CASE(parseX11RGB)
{
	const auto parsedColours = AssetFactory::parseX11RGB(FIXTURESDIR "rgb.txt");
	BOOST_REQUIRE_EQUAL(parsedColours.size(), 20);
	BOOST_CHECK_CLOSE_VEC(parsedColours.at("cyan"), AssetFactory::Colour(0, 1, 1));
	BOOST_CHECK_CLOSE_VEC(parsedColours.at("slategrey"), AssetFactory::Colour(0.44F, 0.5, 0.56F));
	BOOST_CHECK_CLOSE_VEC(parsedColours.at("lightsteelblue1"), AssetFactory::Colour(0.79, 0.88, 1));
}
