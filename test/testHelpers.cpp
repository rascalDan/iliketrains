#include "testHelpers.h"

const std::filesystem::path ANALYSIS_DIRECTORY = []() {
	auto xdgRuntimeDir = getenv("XDG_RUNTIME_DIR");
	return std::filesystem::path {xdgRuntimeDir ? xdgRuntimeDir : "/tmp"} / "ilt-output";
}();
