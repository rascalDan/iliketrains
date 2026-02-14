#include "gfx/gl/instanceVertices.h"
#include "testMainWindow.h"
#include <benchmark/benchmark.h>
#include <random>

namespace {
	struct Instance {
		GlobalPosition3D pos;
		glm::mat3 rot;
	};

	const TestMainWindowAppBase _;

	class Data : public benchmark::Fixture {
	public:
		void
		SetUp(::benchmark::State & state) override
		{
			auto count = static_cast<size_t>(state.range());
			std::mt19937 gen(std::random_device {}());
			std::uniform_int_distribution<GlobalDistance> xyDistrib(0, 1000000);
			std::uniform_int_distribution<GlobalDistance> zDistrib(0, 10000);
			proxies.reserve(count);
			instances.reserve(count);

			while (count--) {
				proxies.emplace_back(instances.acquire(
						GlobalPosition3D {xyDistrib(gen), xyDistrib(gen), zDistrib(gen)}, glm::mat3 {}));
			}
		}

		void
		TearDown(::benchmark::State &) override
		{
			proxies.clear();
		}

		InstanceVertices<Instance> instances;
		std::vector<InstanceVertices<Instance>::InstanceProxy> proxies;
	};

	BENCHMARK_DEFINE_F(Data, partition1)(benchmark::State & state)
	{
		GlobalPosition2D pos {};
		for (auto loop : state) {
			instances.partition([&pos](const auto & instance) {
				return std::abs(instance.pos.x - pos.x) < 5 && std::abs(instance.pos.y - pos.y) < 5;
			});
			pos += GlobalPosition2D {33, 17};
			pos %= 1000000;
		}
	}

	BENCHMARK_DEFINE_F(Data, partition2)(benchmark::State & state)
	{
		GlobalPosition2D pos {};
		for (auto loop : state) {
			instances.partition(
					[&pos](const auto & instance) {
						return std::abs(instance.pos.x - pos.x) < 5;
					},
					[&pos](const auto & instance) {
						return std::abs(instance.pos.y - pos.y) < 5;
					});
			pos += GlobalPosition2D {33, 17};
			pos %= 1000000;
		}
	}
}

BENCHMARK_REGISTER_F(Data, partition1)->Range(0, 1 << 20);
BENCHMARK_REGISTER_F(Data, partition2)->Range(0, 1 << 20);

BENCHMARK_MAIN();
