#include <benchmark/benchmark.h>
#include <game/geoData.h>

namespace {
	const GeoData tm {GeoData::loadFromAsciiGrid(FIXTURESDIR "height/SD19.asc")};

	void
	terrain_findPoint(benchmark::State & state)
	{
		for (auto _ : state) {
			benchmark::DoNotOptimize(tm.findPoint({315555, 495556}));
		}
	}

	void
	terrain_walk(benchmark::State & state)
	{
		const glm::vec2 point {310001, 490000};
		const GeoData::PointFace start {point, tm.findPoint(point)};
		for (auto _ : state) {
			tm.walk(start, {319999, 500000}, [](auto f) {
				benchmark::DoNotOptimize(f);
			});
		}
	}
}

BENCHMARK(terrain_findPoint);
BENCHMARK(terrain_walk);

BENCHMARK_MAIN();
