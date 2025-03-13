#include "game/terrain.h"
#include "gfx/camera.h"
#include "gfx/frustum.h"
#include "gfx/gl/sceneShader.h"
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

	void
	terrainRender(benchmark::State & state)
	{
		Terrain terrain {GeoData::loadFromAsciiGrid(FIXTURESDIR "height/SD19.asc")};
		SceneShader shader;
		Camera cam {terrain.getExtents().min + GlobalPosition3D {0, 0, 10000}, 45.F, 1.F, 1, 10000};
		cam.setForward(::north + ::east);

		for (auto _ : state) {
			terrain.render(shader, cam);
		}
	}
}

BENCHMARK(terrainMeshgen);
BENCHMARK(terrainRender);

BENCHMARK_MAIN();
