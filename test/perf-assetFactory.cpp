#include "assetFactory/assetFactory.h"
#include "assetFactory/factoryMesh.h"
#include "testMainWindow.h"
#include "ui/applicationBase.h"
#include <benchmark/benchmark.h>

static void
brush47xml_load(benchmark::State & state)
{
	for (auto _ : state) {
		benchmark::DoNotOptimize(AssetFactory::loadXML(RESDIR "/brush47.xml"));
	}
}

static void
brush47xml_mesh(benchmark::State & state)
{
	TestMainWindow window;

	const auto mf = AssetFactory::loadXML(RESDIR "/brush47.xml");
	const auto brush47 = mf->assets.at("brush-47");
	for (auto _ : state) {
		std::for_each(brush47->meshes.begin(), brush47->meshes.end(), [](const FactoryMesh::CPtr & factoryMesh) {
			factoryMesh->createMesh();
		});
	}
}

BENCHMARK(brush47xml_load);
BENCHMARK(brush47xml_mesh);

BENCHMARK_MAIN();
