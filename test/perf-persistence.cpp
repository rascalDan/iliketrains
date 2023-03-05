#include "lib/jsonParse-persistence.h"
#include "testMainWindow.h"
#include "testStructures.h"
#include <benchmark/benchmark.h>

template<typename T>
static void
parse_load_object(benchmark::State & state, T &&, const char * path)
{
	for (auto _ : state) {
		std::ifstream in {path};
		benchmark::DoNotOptimize(Persistence::JsonParsePersistence {}.loadState<T>(in));
	}
}

BENCHMARK_CAPTURE(parse_load_object, load_object, std::unique_ptr<TestObject> {}, FIXTURESDIR "json/load_object.json");
BENCHMARK_CAPTURE(parse_load_object, nested, std::unique_ptr<TestObject> {}, FIXTURESDIR "json/nested.json");
BENCHMARK_CAPTURE(parse_load_object, shared_ptr_diff, std::unique_ptr<SharedTestObject> {},
		FIXTURESDIR "json/shared_ptr_diff.json");
BENCHMARK_CAPTURE(parse_load_object, shared_ptr_same, std::unique_ptr<SharedTestObject> {},
		FIXTURESDIR "json/shared_ptr_same.json");

BENCHMARK_MAIN();
