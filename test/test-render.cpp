#define BOOST_TEST_MODULE test_render

#include "testHelpers.h"
#include "testMainWindow.h"
#include "testRenderOutput.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <game/geoData.h>
#include <game/terrain.h>
#include <game/vehicles/railVehicleClass.h>
#include <gfx/gl/sceneRenderer.h>
#include <gfx/models/texture.h>
#include <lib/glArrays.h>
#include <location.hpp>
#include <maths.h>
#include <stream_support.hpp>
#include <ui/applicationBase.h>
#include <ui/window.h>

class TestScene : public SceneProvider {
	RailVehicleClass train {"brush47"};
	Terrain terrain {[]() {
		auto gd = std::make_shared<GeoData>(GeoData::Limits {{0, 0}, {100, 100}});
		gd->generateRandom();
		return gd;
	}()};
	void
	content(const SceneShader & shader) const override
	{
		terrain.render(shader);
		train.render(shader, Location {{52, 50, 2}}, {Location {{52, 56, 2}}, Location {{52, 44, 2}}});
		train.render(shader, Location {{52, 30, 2}}, {Location {{52, 36, 2}}, Location {{52, 24, 2}}});
	}
	void
	lights(const SceneShader &) const override
	{
	}
	void
	shadows(const ShadowMapper & shadowMapper) const override
	{
		terrain.shadows(shadowMapper);
		train.shadows(shadowMapper, Location {{52, 50, 2}}, {Location {{52, 56, 2}}, Location {{52, 44, 2}}});
		train.shadows(shadowMapper, Location {{52, 30, 2}}, {Location {{52, 36, 2}}, Location {{52, 24, 2}}});
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
	ss.camera.setView({-10, -10, 60}, glm::normalize(glm::vec3 {1, 1, -0.5F}));
	const TestScene scene;
	ss.render(scene);
	glDisable(GL_DEBUG_OUTPUT);
	Texture::save(outImage, size, "/tmp/basic.tga");
}

BOOST_AUTO_TEST_CASE(pointlight)
{
	SceneRenderer ss {size, output};
	ss.camera.setView({-10, -10, 60}, glm::normalize(glm::vec3 {1, 1, -0.5F}));
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
			for (int x = 50; x < 100; x += 20) {
				for (int y = 50; y < 2000; y += 20) {
					shader.pointLight.add({x, y, 4}, {1.0, 1.0, 1.0}, 0.1F);
				}
			}
		}
	};
	const PointLightScene scene;
	ss.render(scene);
	glDisable(GL_DEBUG_OUTPUT);
	Texture::save(outImage, size, "/tmp/pointlight.tga");
}

BOOST_AUTO_TEST_CASE(spotlight)
{
	SceneRenderer ss {size, output};
	ss.camera.setView({-10, -10, 60}, glm::normalize(glm::vec3 {1, 1, -0.5F}));
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
			shader.spotLight.add({50, 50, 15}, down, {1.0, 1.0, 1.0}, 0.01F, 1);
			shader.spotLight.add({51, 59.5, 1}, north, {1.0, 1.0, 1.0}, 0.001F, .5);
			shader.spotLight.add({53, 59.5, 1}, north, {1.0, 1.0, 1.0}, 0.001F, .5);
			shader.spotLight.add({60, 50, 3}, north + east, {1.0, 1.0, 1.0}, 0.0001F, .7F);
		}
	};
	const PointLightScene scene;
	ss.render(scene);
	glDisable(GL_DEBUG_OUTPUT);
	Texture::save(outImage, size, "/tmp/spotlight.tga");
}

BOOST_AUTO_TEST_SUITE_END();
