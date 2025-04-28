#include "gfx/gl/instanceVertices.h"
#include "testMainWindow.h"
#include <benchmark/benchmark.h>
#include <random>

namespace {
	struct Instance {
		GlobalPosition3D pos;
		glm::mat3 rot;
	};

	struct Data {
		explicit Data(size_t n)
		{
			std::mt19937 gen(std::random_device {}());
			std::uniform_int_distribution<GlobalDistance> xyDistrib(0, 1000000);
			std::uniform_int_distribution<GlobalDistance> zDistrib(0, 10000);
			while (n--) {
				proxies.emplace_back(instances.acquire(
						GlobalPosition3D {xyDistrib(gen), xyDistrib(gen), zDistrib(gen)}, glm::mat3 {}));
			}
		}

		InstanceVertices<Instance> instances;
		std::vector<InstanceVertices<Instance>::InstanceProxy> proxies;
	};

	void
	partition(benchmark::State & state)
	{
		TestMainWindowAppBase window;
		Data data(static_cast<size_t>(state.range()));
		GlobalPosition2D pos {};
		for (auto loop : state) {
			data.instances.partition([&pos](const auto & instance) {
				return std::abs(instance.pos.x - pos.x) < 5 && std::abs(instance.pos.y - pos.y) < 5;
			});
			pos += GlobalPosition2D {33, 17};
			pos %= 1000000;
		}
	}
}

BENCHMARK(partition)->Range(0, 1 << 20);

BENCHMARK_MAIN();
