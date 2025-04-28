#define ENUM_PROBE
#include "enumDetailsData.h"
#include <enumDetails.h>

// Test type name
static_assert(EnumTypeDetails<GlobalUnscoped>::TYPE_NAME == "GlobalUnscoped");
static_assert(EnumTypeDetails<GlobalScoped>::TYPE_NAME == "GlobalScoped");
static_assert(EnumTypeDetails<ns::Unscoped>::TYPE_NAME == "ns::Unscoped");
static_assert(EnumTypeDetails<ns::Scoped>::TYPE_NAME == "ns::Scoped");

static_assert(EnumValueDetails<GlobalUnscoped::Aa>::VALUE_NAME == "Aa");
static_assert(EnumValueDetails<GlobalScoped::Aa>::VALUE_NAME == "Aa");
static_assert(EnumValueDetails<ns::Unscoped::Aa>::VALUE_NAME == "Aa");
static_assert(EnumValueDetails<ns::Scoped::Aa>::VALUE_NAME == "Aa");

namespace test1 {
	static_assert(EnumValueDetails<DefaultDense::A>::VALID);
	static_assert(EnumValueDetails<DefaultDense::De>::VALID);
	static_assert(EnumValueDetails<static_cast<DefaultDense>(0)>::VALID);
	static_assert(EnumValueDetails<static_cast<DefaultDense>(3)>::VALID);
	static_assert(!EnumValueDetails<static_cast<DefaultDense>(-1)>::VALID);
	static_assert(!EnumValueDetails<static_cast<DefaultDense>(4)>::VALID);
	static_assert(EnumValueDetails<DefaultDense::A>::VALUE_NAME == "A");
	static_assert(EnumValueDetails<DefaultDense::De>::VALUE_NAME == "De");
	using EdDd = EnumDetails<DefaultDense>;
	static_assert(EnumValueCollection<DefaultDense>::Vs::size() == 127);
	static_assert(EdDd::VALID_FLAGS.size() == 127);
	static_assert(EdDd::VALUES.size() == 4);
	static_assert(std::ranges::is_sorted(EdDd::VALUES));
	static_assert(EdDd::VALUES.at(0) == DefaultDense::A);
	static_assert(EdDd::VALUES.at(3) == DefaultDense::De);
	static_assert(EdDd::NAMES.at(0) == "A");
	static_assert(EdDd::NAMES.at(3) == "De");

	static_assert(EdDd::isValid(DefaultDense::A));
	static_assert(EdDd::isValid(DefaultDense::De));
	static_assert(!EdDd::isValid(DefaultDense(-1)));
	static_assert(!EdDd::parse("").has_value());
	static_assert(!EdDd::parse("nonsense").has_value());
	static_assert(EdDd::parse("Bee").value() == DefaultDense::Bee);
	static_assert(EdDd::parse("Ci").value() == DefaultDense::Ci);
	static_assert(EdDd::toString(DefaultDense::De).value() == "De");
	static_assert(!EdDd::toString(static_cast<DefaultDense>(10)).has_value());
}

namespace test2 {
	static_assert(EnumValueDetails<NumberedSparse::Bee>::VALID);
	static_assert(EnumValueDetails<static_cast<NumberedSparse>(0)>::VALID);
	static_assert(EnumValueDetails<static_cast<NumberedSparse>(3)>::VALID);
	static_assert(EnumValueDetails<static_cast<NumberedSparse>(-20)>::VALID);
	static_assert(EnumValueDetails<static_cast<NumberedSparse>(100)>::VALID);
	static_assert(!EnumValueDetails<static_cast<NumberedSparse>(2)>::VALID);
	static_assert(EnumValueDetails<NumberedSparse::A>::VALUE_NAME == "A");
	static_assert(EnumValueDetails<NumberedSparse::De>::VALUE_NAME == "De");
	using EdNs = EnumDetails<NumberedSparse>;
	static_assert(EnumValueCollection<NumberedSparse>::Vs::size() == 7);
	static_assert(EdNs::VALUES.size() == 4);
	static_assert(EdNs::VALID_FLAGS.size() == 7);
	static_assert(std::ranges::is_sorted(EdNs::VALUES));
	static_assert(EdNs::VALUES.at(0) == NumberedSparse::Ci);
	static_assert(EdNs::VALUES.at(1) == NumberedSparse::A);
	static_assert(EdNs::VALUES.at(2) == NumberedSparse::Bee);
	static_assert(EdNs::VALUES.at(3) == NumberedSparse::De);
	static_assert(EdNs::NAMES.at(0) == "Ci");
	static_assert(EdNs::NAMES.at(1) == "A");
	static_assert(EdNs::NAMES.at(2) == "Bee");
	static_assert(EdNs::NAMES.at(3) == "De");

	static_assert(EdNs::isValid(NumberedSparse::A));
	static_assert(EdNs::isValid(NumberedSparse::De));
	static_assert(!EdNs::isValid(NumberedSparse(-1)));
	static_assert(!EdNs::parse("").has_value());
	static_assert(!EdNs::parse("nonsense").has_value());
	static_assert(EdNs::parse("Bee").value() == NumberedSparse::Bee);
	static_assert(EdNs::parse("Ci").value() == NumberedSparse::Ci);
	static_assert(EdNs::toString(NumberedSparse::Ci).value() == "Ci");
	static_assert(!EdNs::toString(static_cast<NumberedSparse>(10)).has_value());
}
