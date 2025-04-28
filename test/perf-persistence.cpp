#include "lib/jsonParse-persistence.h"
#include "testStructures.h"
#include <benchmark/benchmark.h>

namespace {
	template<typename T>
	void
	parseLoadObject(benchmark::State & state, T &&, const char * path)
	{
		for (auto loop : state) {
			std::ifstream inStrm {path};
			benchmark::DoNotOptimize(Persistence::JsonParsePersistence {}.loadState<T>(inStrm));
		}
	}
}

BENCHMARK_CAPTURE(parseLoadObject, load_object, std::unique_ptr<TestObject> {}, FIXTURESDIR "json/load_object.json");
BENCHMARK_CAPTURE(parseLoadObject, nested, std::unique_ptr<TestObject> {}, FIXTURESDIR "json/nested.json");
BENCHMARK_CAPTURE(parseLoadObject, shared_ptr_diff, std::unique_ptr<SharedTestObject> {},
		FIXTURESDIR "json/shared_ptr_diff.json");
BENCHMARK_CAPTURE(parseLoadObject, shared_ptr_same, std::unique_ptr<SharedTestObject> {},
		FIXTURESDIR "json/shared_ptr_same.json");

BENCHMARK_MAIN();
