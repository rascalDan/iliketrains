#define BOOST_TEST_MODULE terrain
#include "testHelpers.h"
#include "testMainWindow.h"
#include "testRenderOutput.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <stream_support.h>

#include <game/terrain.h>
#include <gfx/gl/sceneRenderer.h>

#include <game/geoData.h>

namespace {
	class TestTerrainMesh : public GeoData {
	public:
		TestTerrainMesh() : GeoData {GeoData::loadFromAsciiGrid(FIXTURESDIR "height/SD19.asc")} { }
	};

	constexpr size_t NCOLS = 200, NROWS = 200, XLLCORNER = 310000000, YLLCORNER = 490000000, CELLSIZE = 50000;
	const TestTerrainMesh FIXED_TERRTAIN;
}

BOOST_GLOBAL_FIXTURE(TestMainWindowAppBase);

BOOST_FIXTURE_TEST_SUITE(ttm, TestTerrainMesh);

BOOST_AUTO_TEST_CASE(LoadSuccess)
{
	BOOST_CHECK_EQUAL(NCOLS * NROWS, n_vertices());
	BOOST_CHECK_EQUAL(2 * (NCOLS - 1) * (NROWS - 1), n_faces());
	const auto [lower, upper] = getExtents();
	BOOST_CHECK_EQUAL(lower, GlobalPosition3D(310000000, 490000000, -2600));
	BOOST_CHECK_EQUAL(upper, GlobalPosition3D(319950000, 499950000, 571600));
}

#ifndef NDEBUG
BOOST_AUTO_TEST_CASE(SanityCheck)
{
	BOOST_CHECK_NO_THROW(sanityCheck());
}
#endif

BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_CASE(LocatePointFace)
{
	const GeoData::PointFace pointFace {{310002000, 490003000}};
	BOOST_CHECK(!pointFace.isLocated());
	BOOST_CHECK(pointFace.face(&FIXED_TERRTAIN).is_valid());
	BOOST_CHECK_EQUAL(pointFace.face(&FIXED_TERRTAIN).idx(), 0);
}

BOOST_AUTO_TEST_CASE(PreLocatePointFace)
{
	const GeoData::PointFace pointFace {{310002000, 490003000}, &FIXED_TERRTAIN};
	BOOST_CHECK(pointFace.isLocated());
	BOOST_CHECK_EQUAL(pointFace.face(&FIXED_TERRTAIN).idx(), 0);
}

using FindPointData = std::tuple<GlobalPosition2D, int>;

// No boundary cases as these can produce different valid results depending on starting point
BOOST_DATA_TEST_CASE(FindPointOnTerrain,
		boost::unit_test::data::make<FindPointData>({
				{{0, 0}, -1}, {{XLLCORNER, 0}, -1}, {{0, YLLCORNER}, -1}, {{XLLCORNER + 1, YLLCORNER + 2}, 0},
				{{XLLCORNER + (CELLSIZE * (NROWS - 1)) - 2, YLLCORNER + (CELLSIZE * (NCOLS - 1)) - 1}, 79200},
				{{315555000, 495556000}, 44400}, // perf test target
		})
				* boost::unit_test::data::make<int>(
						{0, 1, 2, 3, 4, 5, 6, 10, 100, 150, 200, 1000, 1234, 17439, 79201, 79200, 79199}),
		point, exp, start)
{
	BOOST_CHECK_EQUAL(exp, FIXED_TERRTAIN.findPoint(point, GeoData::FaceHandle(start)).idx());
}

using FindPositionData = std::tuple<GlobalPosition2D, GlobalDistance>;

BOOST_DATA_TEST_CASE(FindPositionAt,
		boost::unit_test::data::make<FindPositionData>({
				// corners
				{{310000000, 490000000}, 32800},
				{{310050000, 490050000}, 33000},
				{{310000000, 490050000}, 32700},
				{{310050000, 490000000}, 33200},
				{{310750000, 490150000}, 58400},
				// midpoints
				{{310025000, 490025000}, 32900},
				{{310025000, 490050000}, 32850},
				{{310000000, 490025000}, 32750},
				// other
				{{310751000, 490152000}, 58326},
		}),
		point, height)
{
	BOOST_CHECK_EQUAL(FIXED_TERRTAIN.positionAt(point), point || height);
}

using FindRayIntersectData = std::tuple<GlobalPosition3D, Direction3D, GlobalPosition3D>;

BOOST_DATA_TEST_CASE(FindRayIntersect,
		boost::unit_test::data::make<FindRayIntersectData>({
				{{310000000, 490000000, 50000}, {1, 1, -2}, {310008583, 490008583, 32834}},
				{{310000000, 490000000, 50000}, {1, 1, -1}, {310017131, 490017131, 32869}},
		}),
		point, direction, intersectionPoint)
{
	BOOST_CHECK_EQUAL(FIXED_TERRTAIN.intersectRay({point, direction})->first, intersectionPoint);
}

BOOST_AUTO_TEST_CASE(BoundaryWalk)
{
	size_t count {};
	FIXED_TERRTAIN.boundaryWalk([&count](auto heh) {
		BOOST_CHECK(FIXED_TERRTAIN.is_boundary(heh));
		count++;
	});
	BOOST_CHECK_EQUAL(count, 2 * (NCOLS + NROWS - 2));
}

using WalkTerrainData = std::tuple<GlobalPosition2D, GlobalPosition2D, std::vector<int>>;

BOOST_DATA_TEST_CASE(WalkTerrain,
		boost::unit_test::data::make<WalkTerrainData>({
				{{310002000, 490003000}, {310002000, 490003000}, {0}},
				{{310003000, 490002000}, {310003000, 490002000}, {1}},
				{{310002000, 490003000}, {310003000, 490002000}, {0, 1}},
				{{310003000, 490002000}, {310002000, 490003000}, {1, 0}},
				{{310002000, 490003000}, {310202000, 490003000}, {0, 1, 2, 3, 4, 5, 6, 7, 8}},
				{{310202000, 490003000}, {310002000, 490003000}, {8, 7, 6, 5, 4, 3, 2, 1, 0}},
				{{310002000, 490003000}, {310002000, 490203000}, {0, 399, 398, 797, 796, 1195, 1194, 1593, 1592}},
				{{310002000, 490003000}, {309999000, 489999000}, {0}},
				{{309999000, 489999000}, {310002000, 490003000}, {0}},
				{{320002000, 500003000}, {319949000, 499948000}, {79201}},
				{{309999000, 490003000}, {310004000, 489997000}, {0, 1}},
				{{310004000, 489997000}, {309999000, 490003000}, {1, 0}},
		}),
		fromPoint, toPoint, visits)
{
	std::vector<int> visited;
	BOOST_CHECK_NO_THROW(FIXED_TERRTAIN.walk(fromPoint, toPoint, [&visited](auto step) {
		if (!visited.empty()) {
			BOOST_CHECK_EQUAL(step.previous.idx(), visited.back());
		}
		visited.emplace_back(step.current.idx());
	}));
	BOOST_CHECK_EQUAL_COLLECTIONS(visited.begin(), visited.end(), visits.begin(), visits.end());
}

BOOST_DATA_TEST_CASE(WalkTerrainSetsFromFace,
		boost::unit_test::data::make<WalkTerrainData>({
				{{310002000, 490003000}, {310002000, 490003000}, {0}},
				{{310003000, 490002000}, {310003000, 490002000}, {1}},
				{{310002000, 490003000}, {310003000, 490002000}, {0, 1}},
				{{310003000, 490002000}, {310002000, 490003000}, {1, 0}},
		}),
		fromPoint, toPoint, visits)
{
	GeoData::PointFace pointFace {fromPoint};
	BOOST_CHECK_NO_THROW(FIXED_TERRTAIN.walk(pointFace, toPoint, [](auto) { }));
	BOOST_CHECK_EQUAL(pointFace.face(&FIXED_TERRTAIN).idx(), visits.front());
}

BOOST_DATA_TEST_CASE(WalkTerrainUntil,
		boost::unit_test::data::make<WalkTerrainData>({
				{{310002000, 490003000}, {310002000, 490003000}, {0}},
				{{310003000, 490002000}, {310003000, 490002000}, {1}},
				{{310002000, 490003000}, {310003000, 490002000}, {0, 1}},
				{{310003000, 490002000}, {310002000, 490003000}, {1, 0}},
				{{310002000, 490003000}, {310202000, 490003000}, {0, 1, 2, 3, 4}},
				{{310202000, 490003000}, {310002000, 490003000}, {8, 7, 6, 5, 4}},
				{{310002000, 490003000}, {310002000, 490203000}, {0, 399, 398, 797, 796}},
		}),
		fromPoint, toPoint, visits)
{
	std::vector<int> visited;
	BOOST_CHECK_NO_THROW(FIXED_TERRTAIN.walkUntil(fromPoint, toPoint, [&visited](const auto & step) {
		visited.emplace_back(step.current.idx());
		return visited.size() >= 5;
	}));
	BOOST_CHECK_EQUAL_COLLECTIONS(visited.begin(), visited.end(), visits.begin(), visits.end());
}

using WalkTerrainCurveData = std::tuple<GlobalPosition2D, GlobalPosition2D, GlobalPosition2D, std::vector<int>,
		std::vector<GlobalPosition2D>>;

BOOST_TEST_DECORATOR(*boost::unit_test::timeout(1))

BOOST_DATA_TEST_CASE(WalkTerrainCurveSetsFromFace,
		boost::unit_test::data::make<WalkTerrainCurveData>({
				{{310002000, 490003000}, {310002000, 490003000}, {310002000, 490003000}, {0}, {}},
				{{310003000, 490002000}, {310003000, 490002000}, {310003000, 490002000}, {1}, {}},
				{{310202000, 490203000}, {310002000, 490003000}, {310002000, 490203000},
						{1600, 1601, 1202, 1201, 802, 803, 404, 403, 4, 3, 2, 1, 0},
						{
								{310201997, 490201997},
								{310201977, 490200000},
								{310200000, 490174787},
								{310194850, 490150000},
								{310192690, 490142690},
								{310173438, 490100000},
								{310150000, 490068479},
								{310130806, 490050000},
								{310100000, 490028656},
								{310062310, 490012310},
								{310050000, 490008845},
								{310003003, 490003003},
						}},
				{{310999999, 490205000}, {310999999, 490203000}, {310999000, 490204000}, {1631, 1632, 1631},
						{
								{311000000, 490204999},
								{311000000, 490203001},
						}},
		}),
		fromPoint, toPoint, centre, visits, exits)
{
	BOOST_REQUIRE_EQUAL(visits.size(), exits.size() + 1);

	std::vector<int> visited;
	std::vector<GlobalPosition2D> exited;
	BOOST_CHECK_NO_THROW(FIXED_TERRTAIN.walk(fromPoint, toPoint, centre, [&](const auto & step) {
		visited.emplace_back(step.current.idx());
		BOOST_REQUIRE(!std::ranges::contains(exited, step.exitPosition));
		exited.emplace_back(step.exitPosition);
	}));
	BOOST_CHECK_EQUAL_COLLECTIONS(visited.begin(), visited.end(), visits.begin(), visits.end());
	BOOST_CHECK_EQUAL_COLLECTIONS(exited.begin() + 1, exited.end(), exits.begin(), exits.end());
}

using FindEntiesData = std::tuple<GlobalPosition2D, GlobalPosition2D, int>;

BOOST_DATA_TEST_CASE(FindEntryHalfEdge,
		boost::unit_test::data::make<FindEntiesData>({
				{{307739360, 494851616}, {314056992, 500079744}, 160667},
				{{308597952, 498417056}, {315154144, 504671456}, 233623},
				{{302690592, 502270912}, {311585184, 497868064}, 207311},
		}),
		fromPoint, toPoint, heh)
{
	BOOST_CHECK_EQUAL(FIXED_TERRTAIN.findEntry(fromPoint, toPoint).idx(), heh);
}

using DeformTerrainData = std::tuple<std::vector<GlobalPosition3D>,
		std::vector<std::pair<std::pair<GlobalPosition3D, Direction3D>, std::string>>>;

BOOST_TEST_DECORATOR(*boost::unit_test::timeout(2));

BOOST_DATA_TEST_CASE(Deform, loadFixtureJson<DeformTerrainData>("geoData/deform/1.json"), points, cams)
{
	Surface surface;
	surface.colorBias = RGB {0, 0, 1};

	TestRenderOutput tro {{640, 480}};

	struct TestTerrain : public SceneProvider {
		explicit TestTerrain(GeoData geoData) : terrain(std::move(geoData)) { }

		const Terrain terrain;

		void
		content(const SceneShader & shader, const Frustum & frustum) const override
		{
			terrain.render(shader, frustum);
		}

		void
		environment(const SceneShader &, const SceneRenderer & renderer) const override
		{
			renderer.setAmbientLight({0.1, 0.1, 0.1});
			renderer.setDirectionalLight({1, 1, 1}, {{quarter_pi, -3 * half_pi}}, *this);
		}

		void
		lights(const SceneShader &) const override
		{
		}

		void
		shadows(const ShadowMapper & shadowMapper, const Frustum & frustum) const override
		{
			terrain.shadows(shadowMapper, frustum);
		}
	};

	TestTerrain terrain {[&points, &surface]() {
		auto geoData = GeoData::createFlat({0, 0}, {1000000, 1000000}, 100);
		BOOST_CHECK_NO_THROW(geoData.setHeights(points, {.surface = &surface}));
		return geoData;
	}()};
	SceneRenderer renderer {tro.size, tro.output};
	std::for_each(cams.begin(), cams.end(), [&renderer, &terrain, &tro](const auto & cam) {
		renderer.camera.setView(cam.first.first, glm::normalize(cam.first.second));
		BOOST_CHECK_NO_THROW(renderer.render(terrain));
		Texture::save(tro.outImage, cam.second.c_str());
	});
}

BOOST_TEST_DECORATOR(*boost::unit_test::timeout(2));

BOOST_DATA_TEST_CASE(
		deformMulti, loadFixtureJson<std::vector<std::vector<GlobalPosition3D>>>("geoData/deform/multi1.json"), points)
{
	BOOST_REQUIRE(!points.empty());
	Surface surface;
	auto geoData = std::make_shared<GeoData>(GeoData::createFlat({0, 0}, {1000000, 1000000}, 100));
	for (const auto & strip : points) {
		BOOST_REQUIRE_GE(strip.size(), 3);
		BOOST_CHECK_NO_THROW(geoData->setHeights(strip, {.surface = &surface, .nearNodeTolerance = 50}));
	}
}
