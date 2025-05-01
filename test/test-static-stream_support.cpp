#include "stream_support.h"

#include <array>
#include <map>
#include <set>
#include <vector>

static_assert(NonStringIterableCollection<std::vector<int>>);
static_assert(NonStringIterableCollection<std::set<int>>);
static_assert(NonStringIterableCollection<std::map<int, int>>);
static_assert(NonStringIterableCollection<std::vector<char>>);
static_assert(NonStringIterableCollection<std::array<int, 1>>);
static_assert(NonStringIterableCollection<std::array<char, 1>>);
static_assert(!NonStringIterableCollection<std::string>);
static_assert(!NonStringIterableCollection<std::string_view>);

static_assert(requires(std::vector<int> input, std::ostream & strm) { strm << input; });
static_assert(requires(std::array<int, 10> input, std::ostream & strm) { strm << input; });
static_assert(requires(std::set<int> input, std::ostream & strm) { strm << input; });
static_assert(requires(std::map<int, int> input, std::ostream & strm) { strm << input; });
