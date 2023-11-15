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
		auto gd = std::make_shared<GeoData>(GeoData::Limits {{0, 0}, {100, 100}}, 10000);
		gd->generateRandom();
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
		boost::unit_test::data::xrange(0.5F, 30.F, 1.3F) * boost::unit_test::data::xrange(0.5F, 10.F, 0.3F)
				* boost::unit_test::data::xrange(50.F, 500.F, 70.F),
		dist, near, far)
{
	static constexpr glm::vec4 pos {-10, -10, 60, 0};
	const Camera cam {pos, half_pi, 1.F, near, far};

	const auto e = cam.extentsAtDist(dist);

	BOOST_CHECK_CLOSE_VEC(e[0], pos + glm::vec4(-dist, dist, -dist, dist));
	BOOST_CHECK_CLOSE_VEC(e[1], pos + glm::vec4(-dist, dist, dist, dist));
	BOOST_CHECK_CLOSE_VEC(e[2], pos + glm::vec4(dist, dist, -dist, dist));
	BOOST_CHECK_CLOSE_VEC(e[3], pos + glm::vec4(dist, dist, dist, dist));
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
