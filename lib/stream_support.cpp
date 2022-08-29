#include "stream_support.hpp"

#include <array>
#include <vector>

static_assert(spanable<std::vector<int>>);
// static_assert(spanable<std::vector<char>>);
static_assert(spanable<std::array<int, 1>>);
// static_assert(spanable<std::array<char, 1>>);
static_assert(!spanable<std::string>);
static_assert(!spanable<std::string_view>);
