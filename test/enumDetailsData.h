#pragma once
#include <enumDetails.h>

enum GlobalUnscoped { aa, b, c };
enum class GlobalScoped { aa, b, c };

namespace ns {
	enum Unscoped { aa, b, c };
	enum class Scoped { aa, b, c };
}

namespace test1 {
	enum class DefaultDense { a, bee, ci, de };
}

namespace test2 {
	enum class NumberedSparse { a = 0, bee = 3, ci = -20, de = 100 };
}

template<> struct EnumValueCollection<test2::NumberedSparse> {
	// Any ordered integer_sequence which includes all enumeration values
	using Vs = std::integer_sequence<int, -100, -20, 0, 3, 10, 100, 1000>;
};
