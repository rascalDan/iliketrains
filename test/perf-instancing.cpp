#include "gfx/gl/instanceVertices.h"
#include "testMainWindow.h"
#include <benchmark/benchmark.h>
#include <random>

struct Instance {
	GlobalPosition3D pos;
	glm::mat3 rot;
};

struct data {
	explicit data(size_t n)
	{
		std::mt19937 gen(std::random_device {}());
		std::uniform_int_distribution<GlobalDistance> xy(0, 1000000);
		std::uniform_int_distribution<GlobalDistance> z(0, 10000);
		while (n--) {
			proxies.emplace_back(instances.acquire(GlobalPosition3D {xy(gen), xy(gen), z(gen)}, glm::mat3 {}));
		}
	}

	InstanceVertices<Instance> instances;
	std::vector<InstanceVertices<Instance>::InstanceProxy> proxies;
};

static void
partition(benchmark::State & state)
{
	TestMainWindow window;
	data d(static_cast<size_t>(state.range()));
	GlobalPosition2D pos {};
	for (auto _ : state) {
		d.instances.partition([&pos](const auto & i) {
			return std::abs(i.pos.x - pos.x) < 5 && std::abs(i.pos.y - pos.y) < 5;
		});
		pos += GlobalPosition2D {33, 17};
		pos %= 1000000;
	}
}

BENCHMARK(partition)->Range(0, 1 << 20);

BENCHMARK_MAIN();
