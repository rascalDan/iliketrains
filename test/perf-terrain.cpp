#include "game/terrain.h"
#include "testMainWindow.h"
#include <benchmark/benchmark.h>

namespace {
	void
	terrainMeshgen(benchmark::State & state)
	{
		TestMainWindowAppBase window;
		Terrain terrain {GeoData::loadFromAsciiGrid(FIXTURESDIR "height/SD19.asc")};

		for (auto _ : state) {
			terrain.generateMeshes();
		}
	}
}

BENCHMARK(terrainMeshgen);

BENCHMARK_MAIN();
