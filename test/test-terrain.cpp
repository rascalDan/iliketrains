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
