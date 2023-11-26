#define BOOST_TEST_MODULE terrain
#include "testHelpers.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
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

BOOST_AUTO_TEST_CASE(normalsAllPointUp)
{
	BOOST_CHECK_EQUAL(std::count_if(faces_begin(), faces_end(),
							  [this](auto && vh) {
								  return normal(vh).z > 0;
							  }),
			n_faces());
	BOOST_CHECK_EQUAL(std::count_if(vertices_begin(), vertices_end(),
							  [this](auto && vh) {
								  return normal(vh).z > 0;
							  }),
			n_vertices());
}

BOOST_AUTO_TEST_CASE(trianglesContainsPoints)
{
	const auto face = face_handle(0);

	BOOST_TEST_CONTEXT(GeoData::Triangle<2>(this, fv_range(face))) {
		BOOST_CHECK(triangleContainsPoint(GlobalPosition2D {xllcorner, yllcorner}, face));
		BOOST_CHECK(triangleContainsPoint(GlobalPosition2D {xllcorner + cellsize, yllcorner + cellsize}, face));
		BOOST_CHECK(triangleContainsPoint(GlobalPosition2D {xllcorner, yllcorner + cellsize}, face));
		BOOST_CHECK(triangleContainsPoint(GlobalPosition2D {xllcorner + 1, yllcorner + 1}, face));
		BOOST_CHECK(triangleContainsPoint(GlobalPosition2D {xllcorner + 1, yllcorner + 2}, face));
		BOOST_CHECK(!triangleContainsPoint(GlobalPosition2D {xllcorner + 3, yllcorner + 2}, face));
		BOOST_CHECK(!triangleContainsPoint(GlobalPosition2D {xllcorner + cellsize, yllcorner}, face));
	}
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
				{{310750000, 490150000}, 58399},
				// midpoints
				{{310025000, 490025000}, 32900},
				{{310025000, 490050000}, 32850},
				{{310000000, 490025000}, 32750},
				// other
				{{310751000, 490152000}, 58326},
		}),
		p, h)
{
	BOOST_CHECK_EQUAL(fixedTerrtain.positionAt(p), GlobalPosition3D(p, h));
}

using FindRayIntersectData = std::tuple<GlobalPosition3D, Direction3D, GlobalPosition3D>;

BOOST_DATA_TEST_CASE(findRayIntersect,
		boost::unit_test::data::make<FindRayIntersectData>({
				{{310000000, 490000000, 50000}, {1, 1, -2}, {310008585, 490008585, 32834}},
				{{310000000, 490000000, 50000}, {1, 1, -1}, {310017136, 490017136, 32868}},
		}),
		p, d, i)
{
	BOOST_CHECK_EQUAL(fixedTerrtain.intersectRay({p, d}).value(), i);
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
	BOOST_CHECK_NO_THROW(fixedTerrtain.walk(from, to, [&visited](auto fh) {
		visited.emplace_back(fh.idx());
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
	BOOST_CHECK_NO_THROW(fixedTerrtain.walk(pf, to, [](auto) {}));
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
	BOOST_CHECK_NO_THROW(fixedTerrtain.walkUntil(from, to, [&visited](auto fh) {
		visited.emplace_back(fh.idx());
		return visited.size() >= 5;
	}));
	BOOST_CHECK_EQUAL_COLLECTIONS(visited.begin(), visited.end(), visits.begin(), visits.end());
}
