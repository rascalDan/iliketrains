#pragma once

#include <chrono>
#include <ctime>

using TickDuration = std::chrono::duration<float, std::chrono::seconds::period>;
time_t operator""_time_t(const char * iso, size_t);
