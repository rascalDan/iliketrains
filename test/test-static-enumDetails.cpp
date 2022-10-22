#define ENUM_PROBE
#include "enumDetailsData.hpp"
#include <enumDetails.hpp>

// Test type name
static_assert(EnumTypeDetails<GlobalUnscoped>::typeName == "GlobalUnscoped");
static_assert(EnumTypeDetails<GlobalScoped>::typeName == "GlobalScoped");
static_assert(EnumTypeDetails<ns::Unscoped>::typeName == "ns::Unscoped");
static_assert(EnumTypeDetails<ns::Scoped>::typeName == "ns::Scoped");

static_assert(EnumValueDetails<GlobalUnscoped::aa>::valueName == "aa");
static_assert(EnumValueDetails<GlobalScoped::aa>::valueName == "aa");
static_assert(EnumValueDetails<ns::Unscoped::aa>::valueName == "aa");
static_assert(EnumValueDetails<ns::Scoped::aa>::valueName == "aa");

namespace test1 {
	static_assert(EnumValueDetails<DefaultDense::a>::valid);
	static_assert(EnumValueDetails<DefaultDense::de>::valid);
	static_assert(EnumValueDetails<static_cast<DefaultDense>(0)>::valid);
	static_assert(EnumValueDetails<static_cast<DefaultDense>(3)>::valid);
	static_assert(!EnumValueDetails<static_cast<DefaultDense>(-1)>::valid);
	static_assert(!EnumValueDetails<static_cast<DefaultDense>(4)>::valid);
	static_assert(EnumValueDetails<DefaultDense::a>::valueName == "a");
	static_assert(EnumValueDetails<DefaultDense::de>::valueName == "de");
	using ED_DD = EnumDetails<DefaultDense>;
	static_assert(EnumValueCollection<DefaultDense>::Vs::size() == 256);
	static_assert(ED_DD::valid_flags.size() == 256);
	static_assert(ED_DD::values.size() == 4);
	static_assert(std::is_sorted(ED_DD::values.begin(), ED_DD::values.end()));
	static_assert(ED_DD::values.at(0) == DefaultDense::a);
	static_assert(ED_DD::values.at(3) == DefaultDense::de);
	static_assert(ED_DD::names.at(0) == "a");
	static_assert(ED_DD::names.at(3) == "de");

	static_assert(ED_DD::is_valid(DefaultDense::a));
	static_assert(ED_DD::is_valid(DefaultDense::de));
	static_assert(!ED_DD::is_valid(DefaultDense(-1)));
	static_assert(!ED_DD::parse("").has_value());
	static_assert(!ED_DD::parse("nonsense").has_value());
	static_assert(ED_DD::parse("bee").value() == DefaultDense::bee);
	static_assert(ED_DD::parse("ci").value() == DefaultDense::ci);
	static_assert(ED_DD::to_string(DefaultDense::de).value() == "de");
	static_assert(!ED_DD::to_string(static_cast<DefaultDense>(10)).has_value());
}

namespace test2 {
	static_assert(EnumValueDetails<NumberedSparse::bee>::valid);
	static_assert(EnumValueDetails<static_cast<NumberedSparse>(0)>::valid);
	static_assert(EnumValueDetails<static_cast<NumberedSparse>(3)>::valid);
	static_assert(EnumValueDetails<static_cast<NumberedSparse>(-20)>::valid);
	static_assert(EnumValueDetails<static_cast<NumberedSparse>(100)>::valid);
	static_assert(!EnumValueDetails<static_cast<NumberedSparse>(2)>::valid);
	static_assert(EnumValueDetails<NumberedSparse::a>::valueName == "a");
	static_assert(EnumValueDetails<NumberedSparse::de>::valueName == "de");
	using ED_NS = EnumDetails<NumberedSparse>;
	static_assert(EnumValueCollection<NumberedSparse>::Vs::size() == 7);
	static_assert(ED_NS::values.size() == 4);
	static_assert(ED_NS::valid_flags.size() == 7);
	static_assert(std::is_sorted(ED_NS::values.begin(), ED_NS::values.end()));
	static_assert(ED_NS::values.at(0) == NumberedSparse::ci);
	static_assert(ED_NS::values.at(1) == NumberedSparse::a);
	static_assert(ED_NS::values.at(2) == NumberedSparse::bee);
	static_assert(ED_NS::values.at(3) == NumberedSparse::de);
	static_assert(ED_NS::names.at(0) == "ci");
	static_assert(ED_NS::names.at(1) == "a");
	static_assert(ED_NS::names.at(2) == "bee");
	static_assert(ED_NS::names.at(3) == "de");

	static_assert(ED_NS::is_valid(NumberedSparse::a));
	static_assert(ED_NS::is_valid(NumberedSparse::de));
	static_assert(!ED_NS::is_valid(NumberedSparse(-1)));
	static_assert(!ED_NS::parse("").has_value());
	static_assert(!ED_NS::parse("nonsense").has_value());
	static_assert(ED_NS::parse("bee").value() == NumberedSparse::bee);
	static_assert(ED_NS::parse("ci").value() == NumberedSparse::ci);
	static_assert(ED_NS::to_string(NumberedSparse::ci).value() == "ci");
	static_assert(!ED_NS::to_string(static_cast<NumberedSparse>(10)).has_value());
}
