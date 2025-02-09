#include <OpenMesh/Core/IO/MeshIO.hh>
#include <boost/test/data/test_case.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test_suite.hpp>

#include "game/geoData.h"

using GeoMutation = std::function<void(GeoData &)>;
using Something = std::tuple<const char *, GeoMutation, size_t, size_t, size_t>;
BOOST_TEST_DONT_PRINT_LOG_VALUE(GeoMutation);

BOOST_DATA_TEST_CASE(deformLogical,
		boost::unit_test::data::make<Something>({
				{"nochange", [](GeoData &) {}, 16, 33, 18}, // No change base case
				{"simple",
						[](GeoData & geoData) {
							Surface surface;
							// Basic triangle, no crossing, simple case
							geoData.setHeights(std::array<GlobalPosition3D, 3> {{
													   {2000, 8000, 1000},
													   {2000, 4000, 1000},
													   {6000, 8000, 1000},
											   }},
									{.surface = &surface, .nearNodeTolerance = 0});
						},
						19, 42, 24},
				{"simple-cross",
						[](GeoData & geoData) {
							Surface surface;
							// Basic triangle, with crossing, reasonably simple case
							geoData.setHeights(std::array<GlobalPosition3D, 3> {{
													   {2000, 8000, 1000},
													   {3000, 4000, 1000},
													   {4000, 9000, 1000},
											   }},
									{.surface = &surface, .nearNodeTolerance = 0});
						},
						19, 42, 24},
				{"quad-multi-cross",
						[](GeoData & geoData) {
							Surface surface;
							// Basic quad, with crossing, spans into adjacent original triangle, should remove that edge
							geoData.setHeights(std::array<GlobalPosition3D, 4> {{
													   {2000, 8000, 1000},
													   {3000, 4000, 1000},
													   {4000, 9000, 1000},
													   {8000, 2000, 1000},
											   }},
									{.surface = &surface, .nearNodeTolerance = 0});
						},
						20, 45, 26},
		}),
		name, func, expVertices, expEdges, expFaces)
{
	auto geoData = GeoData::createFlat({0, 0}, {30'000, 30'000}, 1000);

	BOOST_REQUIRE_NO_THROW(func(geoData));
	OpenMesh::IO::write_mesh(geoData, std::format("/tmp/mesh-{}.obj", name));

	BOOST_CHECK_EQUAL(geoData.n_vertices(), expVertices);
	BOOST_CHECK_EQUAL(geoData.n_edges(), expEdges);
	BOOST_CHECK_EQUAL(geoData.n_faces(), expFaces);
}
