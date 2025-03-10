#define BOOST_TEST_MODULE test_render

#include "testHelpers.h"
#include "testMainWindow.h"
#include "testRenderOutput.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <assetFactory/assetFactory.h>
#include <game/environment.h>
#include <game/gamestate.h>
#include <game/geoData.h>
#include <game/network/rail.h>
#include <game/scenary/foliage.h>
#include <game/scenary/plant.h>
#include <game/terrain.h>
#include <game/vehicles/railVehicle.h>
#include <game/vehicles/railVehicleClass.h>
#include <game/water.h>
#include <gfx/gl/sceneRenderer.h>
#include <gfx/models/texture.h>
#include <lib/glArrays.h>
#include <location.h>
#include <maths.h>
#include <stream_support.h>
#include <ui/applicationBase.h>
#include <ui/window.h>

class TestScene : public SceneProvider {
	RailVehicleClassPtr brush47rvc;
	std::shared_ptr<RailVehicle> train1, train2;
	RailLinks rail;
	std::shared_ptr<Environment> env = std::make_shared<Environment>();

	std::shared_ptr<Terrain> terrain = std::make_shared<Terrain>(GeoData::createFlat({0, 0}, {1000000, 1000000}, 1));
	Water water {terrain};

public:
	TestScene()
	{
		gameState->assets = AssetFactory::loadAll(RESDIR);
		brush47rvc = std::dynamic_pointer_cast<RailVehicleClass>(gameState->assets.at("brush-47"));
		std::random_device randomdev {};
		std::uniform_real_distribution<Angle> rotationDistribution {0, two_pi};
		std::uniform_int_distribution<GlobalDistance> positionOffsetDistribution {-1500, +1500};
		std::uniform_int_distribution<int> treeDistribution {1, 3};
		std::uniform_int_distribution<int> treeVariantDistribution {1, 4};
		train1 = std::make_shared<RailVehicle>(brush47rvc);
		train1->location.setPosition({52000, 50000, 2000});
		train1->bogies.front().setPosition(train1->bogies.front().position() + train1->location.position());
		train1->bogies.back().setPosition(train1->bogies.back().position() + train1->location.position());
		train2 = std::make_shared<RailVehicle>(brush47rvc);
		train2->location.setPosition({52000, 30000, 2000});
		train2->bogies.front().setPosition(train2->bogies.front().position() + train2->location.position());
		train2->bogies.back().setPosition(train2->bogies.back().position() + train2->location.position());
		for (auto x = 40000; x < 100000; x += 5000) {
			for (auto y = 65000; y < 125000; y += 5000) {
				gameState->world.create<Plant>(
						std::dynamic_pointer_cast<Foliage>(gameState->assets.at(std::format(
								"Tree-{:#02}-{}", treeDistribution(randomdev), treeVariantDistribution(randomdev)))),
						Location {{x + positionOffsetDistribution(randomdev), y + positionOffsetDistribution(randomdev),
										  1},
								{0, rotationDistribution(randomdev), 0}});
			}
		}
		rail.addLinksBetween({42000, 50000, 1000}, {65000, 50000, 1000});
		rail.addLinksBetween({65000, 50000, 1000}, {75000, 45000, 2000});
	}

	void
	content(const SceneShader & shader, const Frustum & frustum) const override
	{
		terrain->render(shader, frustum);
		water.render(shader, frustum);
		rail.render(shader, frustum);
		std::ranges::for_each(gameState->assets, [&shader, &frustum](const auto & asset) {
			if (const auto renderable = std::dynamic_pointer_cast<const Renderable>(asset.second)) {
				renderable->render(shader, frustum);
			}
		});
	}

	void
	lights(const SceneShader &) const override
	{
	}

	void
	environment(const SceneShader &, const SceneRenderer & r) const override
	{
		env->render(r, *this);
	}

	void
	shadows(const ShadowMapper & shadowMapper, const Frustum & frustum) const override
	{
		terrain->shadows(shadowMapper, frustum);
		std::ranges::for_each(gameState->assets, [&shadowMapper, &frustum](const auto & asset) {
			if (const auto renderable = std::dynamic_pointer_cast<const Renderable>(asset.second)) {
				renderable->shadows(shadowMapper, frustum);
			}
		});
	}
};

BOOST_GLOBAL_FIXTURE(TestMainWindowAppBase);

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
	class TestSceneRenderer : public SceneRenderer {
		using SceneRenderer::SceneRenderer;

	public:
		void
		saveBuffers(const std::filesystem::path & prefix) const
		{
			std::filesystem::create_directories(prefix);
			Texture::save(gAlbedoSpec, (prefix / "albedo.tga").c_str());
			Texture::savePosition(gPosition, (prefix / "position.tga").c_str());
			Texture::saveNormal(gNormal, (prefix / "normal.tga").c_str());
			Texture::save(gIllumination, (prefix / "illumination.tga").c_str());
		}
	};

	TestSceneRenderer ss {size, output};
	ss.camera.setView({-10000, -10000, 60000}, glm::normalize(glm::vec3 {1, 1, -0.5F}));
	const TestScene scene;
	ss.render(scene);
	ss.saveBuffers("/tmp/basic");
	Texture::save(outImage, "/tmp/basic/final.tga");
}

BOOST_AUTO_TEST_CASE(terrain)
{
	SceneRenderer ss {size, output};
	ss.camera.setView({310000000, 490000000, 600000}, glm::normalize(glm::vec3 {1, 1, -0.5F}));

	class TestTerrain : public SceneProvider {
		std::shared_ptr<Terrain> terrain
				= std::make_shared<Terrain>(GeoData::loadFromAsciiGrid(FIXTURESDIR "height/SD19.asc"));
		Water water {terrain};

		void
		content(const SceneShader & shader, const Frustum & frustum) const override
		{
			terrain->render(shader, frustum);
			water.render(shader, frustum);
		}

		void
		environment(const SceneShader &, const SceneRenderer & sr) const override
		{
			sr.setAmbientLight({0.1, 0.1, 0.1});
			sr.setDirectionalLight({1, 1, 1}, {{0, quarter_pi}}, *this);
		}

		void
		lights(const SceneShader &) const override
		{
		}

		void
		shadows(const ShadowMapper & shadowMapper, const Frustum & frustum) const override
		{
			terrain->shadows(shadowMapper, frustum);
		}
	};

	ss.render(TestTerrain {});
	Texture::save(outImage, "/tmp/terrain.tga");
}

BOOST_AUTO_TEST_CASE(railnet)
{
	SceneRenderer ss {size, output};
	ss.camera.setView({0, 0, 10000}, glm::normalize(glm::vec3 {1, 1, -0.5F}));

	class TestRail : public SceneProvider {
		RailLinks net;

	public:
		TestRail()
		{
			net.addLinksBetween({20000, 10000, 0}, {100000, 100000, 0});
			net.addLinksBetween({20000, 10000, 0}, {10000, 10000, 0});
			net.addLinksBetween({10000, 20000, 0}, {100000, 120000, 0});
			net.addLinksBetween({10000, 20000, 0}, {10000, 10000, 0});
			net.addLinksBetween({100000, 100000, 0}, {100000, 120000, 0});
		}

		void
		content(const SceneShader & shader, const Frustum & frustum) const override
		{
			net.render(shader, frustum);
		}

		void
		environment(const SceneShader &, const SceneRenderer & sr) const override
		{
			sr.setAmbientLight({0.1, 0.1, 0.1});
			sr.setDirectionalLight({1, 1, 1}, {{0, quarter_pi}}, *this);
		}

		void
		lights(const SceneShader &) const override
		{
		}

		void
		shadows(const ShadowMapper &, const Frustum &) const override
		{
		}
	};

	ss.render(TestRail {});
	Texture::save(outImage, "/tmp/railnet.tga");
}

BOOST_AUTO_TEST_SUITE_END();
