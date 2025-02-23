#include <benchmark/benchmark.h>
#include <game/geoData.h>

namespace {
	const GeoData tm {GeoData::loadFromAsciiGrid(FIXTURESDIR "height/SD19.asc")};

	void
	terrain_findPoint(benchmark::State & state)
	{
		for (auto _ : state) {
			benchmark::DoNotOptimize(tm.findPoint({315555000, 495556000}));
		}
	}

	void
	terrain_walk(benchmark::State & state)
	{
		const glm::vec2 point {310001000, 490000000};
		const GeoData::PointFace start {point, tm.findPoint(point)};
		for (auto _ : state) {
			tm.walk(start, {319999000, 500000000}, [](auto f) {
				benchmark::DoNotOptimize(f);
			});
		}
	}

	void
	terrain_walkBoundary(benchmark::State & state)
	{
		for (auto _ : state) {
			tm.boundaryWalk([](auto heh) {
				benchmark::DoNotOptimize(heh);
			});
		}
	}

	void
	terrain_deform(benchmark::State & state)
	{
		std::array<GlobalPosition3D, 3> points {{
				{315555000, 495556000, 0},
				{315655000, 495556000, 0},
				{315655000, 495557000, 0},
		}};
		for (auto _ : state) {
			auto geoData {tm};
			benchmark::DoNotOptimize(geoData.setHeights(points, GeoData::SetHeightsOpts {.surface = nullptr}));
		}
	}
}

BENCHMARK(terrain_findPoint);
BENCHMARK(terrain_walk);
BENCHMARK(terrain_walkBoundary);
BENCHMARK(terrain_deform);

BENCHMARK_MAIN();
