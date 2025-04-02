#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include "imgui.h" // IWYU pragma: export
#ifdef IMGUI_INTERNAL
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wconversion"
#	pragma GCC diagnostic ignored "-Wsign-conversion"
#	include "imgui_internal.h" // IWYU pragma: export
#	pragma GCC diagnostic pop
#endif
#pragma GCC diagnostic pop
