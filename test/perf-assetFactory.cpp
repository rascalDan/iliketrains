#include "assetFactory/assetFactory.h"
#include "assetFactory/factoryMesh.h"
#include "testMainWindow.h"
#include "ui/applicationBase.h"
#include <benchmark/benchmark.h>

static void
brush47xml_load(benchmark::State & state)
{
	TestMainWindow window;

	for (auto _ : state) {
		benchmark::DoNotOptimize(AssetFactory::loadXML(RESDIR "/brush47.xml"));
	}
}

BENCHMARK(brush47xml_load);

BENCHMARK_MAIN();
