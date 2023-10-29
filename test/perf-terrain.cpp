#include <benchmark/benchmark.h>
#include <game/terrain2.h>

namespace {
	const TerrainMesh tm {FIXTURESDIR "height/SD19.asc"};

	void
	terrain_findPoint(benchmark::State & state)
	{
		for (auto _ : state) {
			benchmark::DoNotOptimize(tm.findPoint({315555, 495556}));
		}
	}
}

BENCHMARK(terrain_findPoint);

BENCHMARK_MAIN();
