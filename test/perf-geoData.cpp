#include <benchmark/benchmark.h>
#include <game/geoData.h>

namespace {
	const GeoData geoDataFixture {GeoData::loadFromAsciiGrid(FIXTURESDIR "height/SD19.asc")};

	void
	terrainFindPoint(benchmark::State & state)
	{
		for (auto loop : state) {
			benchmark::DoNotOptimize(geoDataFixture.findPoint({315555000, 495556000}));
		}
	}

	void
	terrainWalk(benchmark::State & state)
	{
		const glm::vec2 point {310001000, 490000000};
		const GeoData::PointFace start {point, geoDataFixture.findPoint(point)};
		for (auto loop : state) {
			geoDataFixture.walk(start, {319999000, 500000000}, [](auto step) {
				benchmark::DoNotOptimize(step);
			});
		}
	}

	void
	terrainWalkBoundary(benchmark::State & state)
	{
		for (auto loop : state) {
			geoDataFixture.boundaryWalk([](auto heh) {
				benchmark::DoNotOptimize(heh);
			});
		}
	}

	void
	terrainDeform(benchmark::State & state)
	{
		std::array<GlobalPosition3D, 3> points {{
				{315555000, 495556000, 0},
				{315655000, 495556000, 0},
				{315655000, 495557000, 0},
		}};
		for (auto loop : state) {
			auto geoData {geoDataFixture};
			benchmark::DoNotOptimize(geoData.setHeights(points, GeoData::SetHeightsOpts {.surface = nullptr}));
		}
	}
}

BENCHMARK(terrainFindPoint);
BENCHMARK(terrainWalk);
BENCHMARK(terrainWalkBoundary);
BENCHMARK(terrainDeform);

BENCHMARK_MAIN();
