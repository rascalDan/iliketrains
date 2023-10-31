#define BOOST_TEST_MODULE terrain
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <stream_support.h>

#include <game/terrain2.h>

class TestTerrainMesh : public TerrainMesh {
public:
	TestTerrainMesh() : TerrainMesh {FIXTURESDIR "height/SD19.asc"} { }
};

constexpr size_t ncols = 200, nrows = 200, xllcorner = 310000, yllcorner = 490000, cellsize = 50;

BOOST_FIXTURE_TEST_SUITE(ttm, TestTerrainMesh);

BOOST_AUTO_TEST_CASE(loadSuccess)
{
	BOOST_CHECK_EQUAL(ncols * nrows, n_vertices());
	BOOST_CHECK_EQUAL(2 * (ncols - 1) * (nrows - 1), n_faces());
}

BOOST_AUTO_TEST_CASE(normalsAllPointUp)
{
	BOOST_CHECK(std::all_of(faces_begin(), faces_end(), [this](auto && vh) {
		return normal(vh).z > 0;
	}));
	BOOST_CHECK(std::all_of(vertices_begin(), vertices_end(), [this](auto && vh) {
		return normal(vh).z > 0;
	}));
}

BOOST_AUTO_TEST_CASE(trianglesContainsPoints)
{
	const auto face = face_handle(0);
	auto vertices = cfv_iter(face);

	BOOST_TEST_CONTEXT(point(*vertices++) << point(*vertices++) << point(*vertices++)) {
		BOOST_CHECK(triangleContainsPoint(glm::vec2 {xllcorner, yllcorner}, face));
		BOOST_CHECK(triangleContainsPoint(glm::vec2 {xllcorner + cellsize, yllcorner + cellsize}, face));
		BOOST_CHECK(triangleContainsPoint(glm::vec2 {xllcorner, yllcorner + cellsize}, face));
		BOOST_CHECK(triangleContainsPoint(glm::vec2 {xllcorner + 1, yllcorner + 1}, face));
		BOOST_CHECK(triangleContainsPoint(glm::vec2 {xllcorner + 1, yllcorner + 2}, face));
		BOOST_CHECK(!triangleContainsPoint(glm::vec2 {xllcorner + 3, yllcorner + 2}, face));
		BOOST_CHECK(!triangleContainsPoint(glm::vec2 {xllcorner + cellsize, yllcorner}, face));
	}
}

BOOST_AUTO_TEST_CASE(locatePointFace)
{
	const PointFace pf {{310002, 490003}};
	BOOST_CHECK(!pf.face.is_valid());
	BOOST_CHECK(locate(pf));
	BOOST_CHECK(pf.face.is_valid());
	BOOST_CHECK_EQUAL(pf.face.idx(), 0);
}

BOOST_AUTO_TEST_SUITE_END();

using FindPointData = std::tuple<glm::vec2, int>;

static const TestTerrainMesh fixedTerrtain;

// No boundary cases as these can produce different valid results depending on starting point
BOOST_DATA_TEST_CASE(findPointOnTerrain,
		boost::unit_test::data::make<FindPointData>({
				{{0, 0}, -1}, {{xllcorner, 0}, -1}, {{0, yllcorner}, -1}, {{xllcorner + 1, yllcorner + 2}, 0},
				{{xllcorner + (cellsize * (nrows - 1)) - 2, yllcorner + (cellsize * (ncols - 1)) - 1}, 79200},
				{{315555, 495556}, 44400}, // perf test target
		})
				* boost::unit_test::data::make<int>(
						{0, 1, 2, 3, 4, 5, 6, 10, 100, 150, 200, 1000, 1234, 17439, 79201, 79200, 79199}),
		p, fh, start)
{
	BOOST_CHECK_EQUAL(fh, fixedTerrtain.findPoint(p, TerrainMesh::FaceHandle(start)).idx());
}

using WalkTerrainData = std::tuple<glm::vec2, glm::vec2, std::vector<int>>;

BOOST_DATA_TEST_CASE(walkTerrain,
		boost::unit_test::data::make<WalkTerrainData>({
				{{310002, 490003}, {310002, 490003}, {0}},
				{{310003, 490002}, {310003, 490002}, {1}},
				{{310002, 490003}, {310003, 490002}, {0, 1}},
				{{310003, 490002}, {310002, 490003}, {1, 0}},
				{{310002, 490003}, {310202, 490003}, {0, 1, 2, 3, 4, 5, 6, 7, 8}},
				{{310202, 490003}, {310002, 490003}, {8, 7, 6, 5, 4, 3, 2, 1, 0}},
				{{310002, 490003}, {310002, 490203}, {0, 399, 398, 797, 796, 1195, 1194, 1593, 1592}},
		}),
		from, to, visits)
{
	std::vector<int> visited;
	BOOST_CHECK_NO_THROW(fixedTerrtain.walk(from, to, [&visited](auto fh) {
		visited.emplace_back(fh.idx());
	}));
	BOOST_CHECK_EQUAL_COLLECTIONS(visited.begin(), visited.end(), visits.begin(), visits.end());
}

BOOST_DATA_TEST_CASE(walkTerrainUntil,
		boost::unit_test::data::make<WalkTerrainData>({
				{{310002, 490003}, {310002, 490003}, {0}},
				{{310003, 490002}, {310003, 490002}, {1}},
				{{310002, 490003}, {310003, 490002}, {0, 1}},
				{{310003, 490002}, {310002, 490003}, {1, 0}},
				{{310002, 490003}, {310202, 490003}, {0, 1, 2, 3, 4}},
				{{310202, 490003}, {310002, 490003}, {8, 7, 6, 5, 4}},
				{{310002, 490003}, {310002, 490203}, {0, 399, 398, 797, 796}},
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
