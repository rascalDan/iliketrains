#include <benchmark/benchmark.h>
#include <game/geoData.h>

namespace {
	const GeoData GEO_DATA_FIXTURE {GeoData::loadFromAsciiGrid(FIXTURESDIR "height/SD19.asc")};

	void
	terrainFindPoint(benchmark::State & state)
	{
		for (auto loop : state) {
			benchmark::DoNotOptimize(GEO_DATA_FIXTURE.findPoint({315555000, 495556000}));
		}
	}

	void
	terrainWalk(benchmark::State & state)
	{
		const glm::vec2 point {310001000, 490000000};
		const GeoData::PointFace start {point, GEO_DATA_FIXTURE.findPoint(point)};
		for (auto loop : state) {
			GEO_DATA_FIXTURE.walk(start, {319999000, 500000000}, [](auto step) {
				benchmark::DoNotOptimize(step);
			});
		}
	}

	void
	terrainWalkBoundary(benchmark::State & state)
	{
		for (auto loop : state) {
			GEO_DATA_FIXTURE.boundaryWalk([](auto heh) {
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
			auto geoData {GEO_DATA_FIXTURE};
			benchmark::DoNotOptimize(geoData.setHeights(points, GeoData::SetHeightsOpts {.surface = nullptr}));
		}
	}
}

BENCHMARK(terrainFindPoint);
BENCHMARK(terrainWalk);
BENCHMARK(terrainWalkBoundary);
BENCHMARK(terrainDeform);

BENCHMARK_MAIN();
