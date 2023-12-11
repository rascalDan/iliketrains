#define BOOST_TEST_MODULE test_render

#include "testHelpers.h"
#include "testMainWindow.h"
#include "testRenderOutput.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <assetFactory/assetFactory.h>
#include <game/geoData.h>
#include <game/terrain.h>
#include <game/vehicles/railVehicle.h>
#include <game/vehicles/railVehicleClass.h>
#include <gfx/gl/sceneRenderer.h>
#include <gfx/models/texture.h>
#include <lib/glArrays.h>
#include <location.h>
#include <maths.h>
#include <stream_support.h>
#include <ui/applicationBase.h>
#include <ui/window.h>

class TestScene : public SceneProvider {
	const RailVehicleClassPtr brush47rvc = std::dynamic_pointer_cast<RailVehicleClass>(
			AssetFactory::loadXML(RESDIR "/brush47.xml")->assets.at("brush-47"));
	std::shared_ptr<RailVehicle> train1, train2;

	Terrain terrain {[]() {
		auto gd = std::make_shared<GeoData>(GeoData::createFlat({0, 0}, {1000000, 1000000}, 1));
		return gd;
	}()};

public:
	TestScene()
	{
		train1 = std::make_shared<RailVehicle>(brush47rvc);
		train1->location.setPosition({52000, 50000, 2000});
		train1->bogies.front().setPosition(train1->bogies.front().position() + train1->location.position());
		train1->bogies.back().setPosition(train1->bogies.back().position() + train1->location.position());
		train2 = std::make_shared<RailVehicle>(brush47rvc);
		train2->location.setPosition({52000, 30000, 2000});
		train2->bogies.front().setPosition(train2->bogies.front().position() + train2->location.position());
		train2->bogies.back().setPosition(train2->bogies.back().position() + train2->location.position());
	}

	void
	content(const SceneShader & shader) const override
	{
		terrain.render(shader);
		brush47rvc->render(shader);
	}

	void
	lights(const SceneShader &) const override
	{
	}

	void
	shadows(const ShadowMapper & shadowMapper) const override
	{
		terrain.shadows(shadowMapper);
		brush47rvc->shadows(shadowMapper);
	}
};

BOOST_GLOBAL_FIXTURE(ApplicationBase);
BOOST_GLOBAL_FIXTURE(TestMainWindow);

BOOST_DATA_TEST_CASE(cam,
		boost::unit_test::data::xrange(500, 30000, 1300) * boost::unit_test::data::xrange(500, 10000, 300)
				* boost::unit_test::data::xrange(50000, 500000, 70000),
		dist, near, far)
{
	static constexpr GlobalPosition4D pos {-10, -10, 60000, 0};
	const Camera cam {pos, half_pi, 1.F, near, far};

	const auto e = cam.extentsAtDist(dist);

	BOOST_CHECK_CLOSE_VECI(e[0], pos + GlobalPosition4D(-dist, dist, -dist, dist));
	BOOST_CHECK_CLOSE_VECI(e[1], pos + GlobalPosition4D(-dist, dist, dist, dist));
	BOOST_CHECK_CLOSE_VECI(e[2], pos + GlobalPosition4D(dist, dist, -dist, dist));
	BOOST_CHECK_CLOSE_VECI(e[3], pos + GlobalPosition4D(dist, dist, dist, dist));
}

BOOST_AUTO_TEST_CASE(camSeaFloor)
{
	const Camera cam {{100, 200, 300}, half_pi, 1.F, 100, 2000};

	const auto e = cam.extentsAtDist(2000);

	BOOST_CHECK_CLOSE_VECI(e[0], GlobalPosition4D(-1700, 2000, -1500, 1800));
	BOOST_CHECK_CLOSE_VECI(e[1], GlobalPosition4D(-1900, 2200, 2300, 2000));
	BOOST_CHECK_CLOSE_VECI(e[2], GlobalPosition4D(1900, 2000, -1500, 1800));
	BOOST_CHECK_CLOSE_VECI(e[3], GlobalPosition4D(2100, 2200, 2300, 2000));
}

BOOST_FIXTURE_TEST_SUITE(w, TestRenderOutput);

BOOST_AUTO_TEST_CASE(basic)
{
	SceneRenderer ss {size, output};
	ss.camera.setView({-10000, -10000, 60000}, glm::normalize(glm::vec3 {1, 1, -0.5F}));
	const TestScene scene;
	ss.render(scene);
	Texture::save(outImage, "/tmp/basic.tga");
}

BOOST_AUTO_TEST_CASE(terrain)
{
	SceneRenderer ss {size, output};
	ss.camera.setView({310000000, 490000000, 600000}, glm::normalize(glm::vec3 {1, 1, -0.5F}));

	class TestTerrain : public SceneProvider {
		Terrain terrain {std::make_shared<GeoData>(GeoData::loadFromAsciiGrid(FIXTURESDIR "height/SD19.asc"))};

		void
		content(const SceneShader & shader) const override
		{
			terrain.render(shader);
		}

		void
		environment(const SceneShader &, const SceneRenderer & sr) const override
		{
			sr.setAmbientLight({0.1, 0.1, 0.1});
			sr.setDirectionalLight({1, 1, 1}, south + down, *this);
		}

		void
		lights(const SceneShader &) const override
		{
		}

		void
		shadows(const ShadowMapper & shadowMapper) const override
		{
			terrain.shadows(shadowMapper);
		}
	};

	ss.render(TestTerrain {});
	Texture::save(outImage, "/tmp/terrain.tga");
}

BOOST_AUTO_TEST_CASE(pointlight)
{
	SceneRenderer ss {size, output};
	ss.camera.setView({-10000, -10000, 60000}, glm::normalize(glm::vec3 {1, 1, -0.5F}));

	class PointLightScene : public TestScene {
	public:
		void
		environment(const SceneShader &, const SceneRenderer & r) const override
		{
			r.setAmbientLight({0.2F, 0.2F, 0.2F});
			r.setDirectionalLight({0.2F, 0.2F, 0.2F}, west + down, *this);
		}

		void
		lights(const SceneShader & shader) const override
		{
			for (int x = 50000; x < 100000; x += 20000) {
				for (int y = 50000; y < 2000000; y += 20000) {
					shader.pointLight.add({x, y, 4000}, {1.0, 1.0, 1.0}, 0.1F);
				}
			}
		}
	};

	const PointLightScene scene;
	ss.render(scene);
	Texture::save(outImage, "/tmp/pointlight.tga");
}

BOOST_AUTO_TEST_CASE(spotlight)
{
	SceneRenderer ss {size, output};
	ss.camera.setView({-10000, -10000, 60000}, glm::normalize(glm::vec3 {1, 1, -0.5F}));

	class PointLightScene : public TestScene {
	public:
		void
		environment(const SceneShader &, const SceneRenderer & r) const override
		{
			r.setAmbientLight({0.2F, 0.2F, 0.2F});
			r.setDirectionalLight({0.2F, 0.2F, 0.2F}, west + down, *this);
		}

		void
		lights(const SceneShader & shader) const override
		{
			shader.spotLight.add({50000, 50000, 15000}, down, {1.0, 1.0, 1.0}, 0.01F, 1);
			shader.spotLight.add({51000, 59500, 1000}, north, {1.0, 1.0, 1.0}, 0.001F, .5);
			shader.spotLight.add({53000, 59500, 1000}, north, {1.0, 1.0, 1.0}, 0.001F, .5);
			shader.spotLight.add({60000, 50000, 3000}, north + east, {1.0, 1.0, 1.0}, 0.0001F, .7F);
		}
	};

	const PointLightScene scene;
	ss.render(scene);
	Texture::save(outImage, "/tmp/spotlight.tga");
}

BOOST_AUTO_TEST_SUITE_END();
