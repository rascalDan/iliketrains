#include "assetFactory/assetFactory.h"
#include "testMainWindow.h"
#include <benchmark/benchmark.h>

static void
brush47xml_load(benchmark::State & state)
{
	TestMainWindowAppBase window;

	for (auto _ : state) {
		benchmark::DoNotOptimize(AssetFactory::loadXML(RESDIR "/brush47.xml"));
	}
}

static void
foliagexml_load(benchmark::State & state)
{
	TestMainWindowAppBase window;

	for (auto _ : state) {
		benchmark::DoNotOptimize(AssetFactory::loadXML(RESDIR "/foliage.xml"));
	}
}

BENCHMARK(brush47xml_load);
BENCHMARK(foliagexml_load);

BENCHMARK_MAIN();
