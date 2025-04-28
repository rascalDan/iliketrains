#include "assetFactory/assetFactory.h"
#include "testMainWindow.h"
#include <benchmark/benchmark.h>

namespace {
	void
	brush47xmlLoad(benchmark::State & state)
	{
		TestMainWindowAppBase window;

		for (auto loop : state) {
			benchmark::DoNotOptimize(AssetFactory::loadXML(RESDIR "/brush47.xml"));
		}
	}

	void
	foliagexmlLoad(benchmark::State & state)
	{
		TestMainWindowAppBase window;

		for (auto loop : state) {
			benchmark::DoNotOptimize(AssetFactory::loadXML(RESDIR "/foliage.xml"));
		}
	}
}

BENCHMARK(brush47xmlLoad);
BENCHMARK(foliagexmlLoad);

BENCHMARK_MAIN();
