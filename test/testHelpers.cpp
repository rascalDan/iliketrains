#include "testHelpers.h"

const std::filesystem::path ANALYSIS_DIRECTORY = []() {
	auto xdgRuntimeDir = getenv("XDG_RUNTIME_DIR");
	auto out = std::filesystem::path {xdgRuntimeDir ? xdgRuntimeDir : "/tmp"} / "ilt-output";
	std::filesystem::create_directories(out);
	return out;
}();
