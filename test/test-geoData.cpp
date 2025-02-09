#define BOOST_TEST_MODULE terrain
#include "game/terrain.h"
#include "test/testMainWindow.h"
#include "test/testRenderOutput.h"
#include "testHelpers.h"
#include "ui/applicationBase.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <gfx/gl/sceneRenderer.h>
#include <stream_support.h>

#include <game/geoData.h>

class TestTerrainMesh : public GeoData {
public:
	TestTerrainMesh() : GeoData {GeoData::loadFromAsciiGrid(FIXTURESDIR "height/SD19.asc")} { }
};

constexpr size_t ncols = 200, nrows = 200, xllcorner = 310000000, yllcorner = 490000000, cellsize = 50000;

BOOST_FIXTURE_TEST_SUITE(ttm, TestTerrainMesh);

BOOST_AUTO_TEST_CASE(loadSuccess)
{
	BOOST_CHECK_EQUAL(ncols * nrows, n_vertices());
	BOOST_CHECK_EQUAL(2 * (ncols - 1) * (nrows - 1), n_faces());
	const auto [lower, upper] = getExtents();
	BOOST_CHECK_EQUAL(lower, GlobalPosition3D(310000000, 490000000, -2600));
	BOOST_CHECK_EQUAL(upper, GlobalPosition3D(319950000, 499950000, 571600));
}

BOOST_AUTO_TEST_CASE(sanityCheck)
{
	BOOST_CHECK_NO_THROW(sanityCheck());
}

BOOST_AUTO_TEST_SUITE_END();

static const TestTerrainMesh fixedTerrtain;

BOOST_AUTO_TEST_CASE(locatePointFace)
{
	const GeoData::PointFace pf {{310002000, 490003000}};
	BOOST_CHECK(!pf.isLocated());
	BOOST_CHECK(pf.face(&fixedTerrtain).is_valid());
	BOOST_CHECK_EQUAL(pf.face(&fixedTerrtain).idx(), 0);
}

BOOST_AUTO_TEST_CASE(preLocatePointFace)
{
	const GeoData::PointFace pf {{310002000, 490003000}, &fixedTerrtain};
	BOOST_CHECK(pf.isLocated());
	BOOST_CHECK_EQUAL(pf.face(&fixedTerrtain).idx(), 0);
}

using FindPointData = std::tuple<GlobalPosition2D, int>;

// No boundary cases as these can produce different valid results depending on starting point
BOOST_DATA_TEST_CASE(findPointOnTerrain,
		boost::unit_test::data::make<FindPointData>({
				{{0, 0}, -1}, {{xllcorner, 0}, -1}, {{0, yllcorner}, -1}, {{xllcorner + 1, yllcorner + 2}, 0},
				{{xllcorner + (cellsize * (nrows - 1)) - 2, yllcorner + (cellsize * (ncols - 1)) - 1}, 79200},
				{{315555000, 495556000}, 44400}, // perf test target
		})
				* boost::unit_test::data::make<int>(
						{0, 1, 2, 3, 4, 5, 6, 10, 100, 150, 200, 1000, 1234, 17439, 79201, 79200, 79199}),
		p, fh, start)
{
	BOOST_CHECK_EQUAL(fh, fixedTerrtain.findPoint(p, GeoData::FaceHandle(start)).idx());
}

using FindPositionData = std::tuple<GlobalPosition2D, GlobalDistance>;

BOOST_DATA_TEST_CASE(findPositionAt,
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
		p, h)
{
	BOOST_CHECK_EQUAL(fixedTerrtain.positionAt(p), p || h);
}

using FindRayIntersectData = std::tuple<GlobalPosition3D, Direction3D, GlobalPosition3D>;

BOOST_DATA_TEST_CASE(findRayIntersect,
		boost::unit_test::data::make<FindRayIntersectData>({
				{{310000000, 490000000, 50000}, {1, 1, -2}, {310008583, 490008583, 32834}},
				{{310000000, 490000000, 50000}, {1, 1, -1}, {310017131, 490017131, 32869}},
		}),
		p, d, i)
{
	BOOST_CHECK_EQUAL(fixedTerrtain.intersectRay({p, d})->first, i);
}

BOOST_AUTO_TEST_CASE(boundaryWalk)
{
	size_t count {};
	fixedTerrtain.boundaryWalk([&count](auto heh) {
		BOOST_CHECK(fixedTerrtain.is_boundary(heh));
		count++;
	});
	BOOST_CHECK_EQUAL(count, 2 * (ncols + nrows - 2));
}

using WalkTerrainData = std::tuple<GlobalPosition2D, GlobalPosition2D, std::vector<int>>;

BOOST_DATA_TEST_CASE(walkTerrain,
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
		from, to, visits)
{
	std::vector<int> visited;
	BOOST_CHECK_NO_THROW(fixedTerrtain.walk(from, to, [&visited](auto step) {
		if (!visited.empty()) {
			BOOST_CHECK_EQUAL(step.previous.idx(), visited.back());
		}
		visited.emplace_back(step.current.idx());
	}));
	BOOST_CHECK_EQUAL_COLLECTIONS(visited.begin(), visited.end(), visits.begin(), visits.end());
}

BOOST_DATA_TEST_CASE(walkTerrainSetsFromFace,
		boost::unit_test::data::make<WalkTerrainData>({
				{{310002000, 490003000}, {310002000, 490003000}, {0}},
				{{310003000, 490002000}, {310003000, 490002000}, {1}},
				{{310002000, 490003000}, {310003000, 490002000}, {0, 1}},
				{{310003000, 490002000}, {310002000, 490003000}, {1, 0}},
		}),
		from, to, visits)
{
	GeoData::PointFace pf {from};
	BOOST_CHECK_NO_THROW(fixedTerrtain.walk(pf, to, [](auto) { }));
	BOOST_CHECK_EQUAL(pf.face(&fixedTerrtain).idx(), visits.front());
}

BOOST_DATA_TEST_CASE(walkTerrainUntil,
		boost::unit_test::data::make<WalkTerrainData>({
				{{310002000, 490003000}, {310002000, 490003000}, {0}},
				{{310003000, 490002000}, {310003000, 490002000}, {1}},
				{{310002000, 490003000}, {310003000, 490002000}, {0, 1}},
				{{310003000, 490002000}, {310002000, 490003000}, {1, 0}},
				{{310002000, 490003000}, {310202000, 490003000}, {0, 1, 2, 3, 4}},
				{{310202000, 490003000}, {310002000, 490003000}, {8, 7, 6, 5, 4}},
				{{310002000, 490003000}, {310002000, 490203000}, {0, 399, 398, 797, 796}},
		}),
		from, to, visits)
{
	std::vector<int> visited;
	BOOST_CHECK_NO_THROW(fixedTerrtain.walkUntil(from, to, [&visited](const auto & step) {
		visited.emplace_back(step.current.idx());
		return visited.size() >= 5;
	}));
	BOOST_CHECK_EQUAL_COLLECTIONS(visited.begin(), visited.end(), visits.begin(), visits.end());
}

using WalkTerrainCurveData = std::tuple<GlobalPosition2D, GlobalPosition2D, GlobalPosition2D, std::vector<int>,
		std::vector<GlobalPosition2D>>;

BOOST_TEST_DECORATOR(*boost::unit_test::timeout(1))

BOOST_DATA_TEST_CASE(walkTerrainCurveSetsFromFace,
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
		from, to, centre, visits, exits)
{
	BOOST_REQUIRE_EQUAL(visits.size(), exits.size() + 1);

	std::vector<int> visited;
	std::vector<GlobalPosition2D> exited;
	BOOST_CHECK_NO_THROW(fixedTerrtain.walk(from, to, centre, [&](const auto & step) {
		visited.emplace_back(step.current.idx());
		BOOST_REQUIRE(!std::ranges::contains(exited, step.exitPosition));
		exited.emplace_back(step.exitPosition);
	}));
	BOOST_CHECK_EQUAL_COLLECTIONS(visited.begin(), visited.end(), visits.begin(), visits.end());
	BOOST_CHECK_EQUAL_COLLECTIONS(exited.begin() + 1, exited.end(), exits.begin(), exits.end());
}

using FindEntiesData = std::tuple<GlobalPosition2D, GlobalPosition2D, int>;

BOOST_DATA_TEST_CASE(findEntries,
		boost::unit_test::data::make<FindEntiesData>({
				{{307739360, 494851616}, {314056992, 500079744}, 160667},
				{{308597952, 498417056}, {315154144, 504671456}, 233623},
				{{302690592, 502270912}, {311585184, 497868064}, 207311},
		}),
		from, to, heh)
{
	BOOST_CHECK_EQUAL(fixedTerrtain.findEntry(from, to).idx(), heh);
}

using DeformTerrainData = std::tuple<std::vector<GlobalPosition3D>,
		std::vector<std::pair<std::pair<GlobalPosition3D, Direction3D>, std::string>>>;

BOOST_TEST_DECORATOR(*boost::unit_test::timeout(2));

BOOST_DATA_TEST_CASE(deform, loadFixtureJson<DeformTerrainData>("geoData/deform/1.json"), points, cams)
{
	Surface surface;
	surface.colorBias = RGB {0, 0, 1};

	ApplicationBase ab;
	TestMainWindow tmw;
	TestRenderOutput tro {{640, 480}};

	struct TestTerrain : public SceneProvider {
		explicit TestTerrain(GeoData gd) : terrain(std::move(gd)) { }

		const Terrain terrain;

		void
		content(const SceneShader & shader) const override
		{
			terrain.render(shader);
		}

		void
		environment(const SceneShader &, const SceneRenderer & sr) const override
		{
			sr.setAmbientLight({0.1, 0.1, 0.1});
			sr.setDirectionalLight({1, 1, 1}, {{quarter_pi, -3 * half_pi}}, *this);
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

	TestTerrain t {[&points, &surface]() {
		auto gd = GeoData::createFlat({0, 0}, {1000000, 1000000}, 100);
		BOOST_CHECK_NO_THROW(gd.setHeights(points, {.surface = &surface}));
		return gd;
	}()};
	SceneRenderer ss {tro.size, tro.output};
	std::for_each(cams.begin(), cams.end(), [&ss, &t, &tro](const auto & cam) {
		ss.camera.setView(cam.first.first, glm::normalize(cam.first.second));
		BOOST_CHECK_NO_THROW(ss.render(t));
		Texture::save(tro.outImage, cam.second.c_str());
	});
}

BOOST_DATA_TEST_CASE(
		deformMulti, loadFixtureJson<std::vector<std::vector<GlobalPosition3D>>>("geoData/deform/multi1.json"), points)
{
	BOOST_REQUIRE(!points.empty());
	Surface surface;
	auto gd = std::make_shared<GeoData>(GeoData::createFlat({0, 0}, {1000000, 1000000}, 100));
	for (const auto & strip : points) {
		BOOST_REQUIRE_GE(strip.size(), 3);
		BOOST_CHECK_NO_THROW(gd->setHeights(strip, {.surface = &surface, .nearNodeTolerance = 50}));
	}
}
