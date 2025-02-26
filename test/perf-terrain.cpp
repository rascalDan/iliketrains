#include "game/terrain.h"
#include "testMainWindow.h"
#include <benchmark/benchmark.h>

namespace {
	const TestMainWindowAppBase window;

	void
	terrainMeshgen(benchmark::State & state)
	{
		Terrain terrain {GeoData::loadFromAsciiGrid(FIXTURESDIR "height/SD19.asc")};

		for (auto _ : state) {
			terrain.generateMeshes();
		}
	}
}

BENCHMARK(terrainMeshgen);

BENCHMARK_MAIN();
