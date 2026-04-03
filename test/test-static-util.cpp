#include "util.h"

namespace {
	struct Base1 {
		int a;
		float b;
	};

	struct Base2 {
		int x;
		float y;
	};

	struct Sub : Base1, Base2 {
		double value;
	};

	static_assert(std::is_same_v<MemberValueType<&Base1::a>, int>);
	static_assert(std::is_same_v<MemberValueType<&Base2::y>, float>);
	static_assert(std::is_same_v<MemberValueType<&Sub::a>, int>);
	static_assert(std::is_same_v<MemberValueType<&Sub::y>, float>);
	static_assert(std::is_same_v<MemberValueType<&Sub::value>, double>);

	static_assert(std::is_same_v<ContainerType<&Base1::a>, Base1>);
	static_assert(std::is_same_v<ContainerType<&Base2::y>, Base2>);
	static_assert(std::is_same_v<ContainerType<&Sub::a>, Base1>);
	static_assert(std::is_same_v<ContainerType<&Sub::y>, Base2>);
	static_assert(std::is_same_v<ContainerType<&Sub::value>, Sub>);
}
