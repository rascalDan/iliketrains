#pragma once
#include <cstdint>
#include <enumDetails.h>

enum GlobalUnscoped : uint8_t { Aa, B, C };
enum class GlobalScoped : int8_t { Aa, B, C };

namespace ns {
	enum Unscoped : int8_t { Aa, B, C };
	enum class Scoped : int8_t { Aa, B, C };
}

namespace test1 {
	enum class DefaultDense : int8_t { A, Bee, Ci, De };
}

namespace test2 {
	enum class NumberedSparse : int8_t { A = 0, Bee = 3, Ci = -20, De = 100 };
}

template<> struct EnumValueCollection<test2::NumberedSparse> {
	// Any ordered integer_sequence which includes all enumeration values
	using Vs = std::integer_sequence<int8_t, -100, -20, 0, 3, 10, 100, 110>;
};
