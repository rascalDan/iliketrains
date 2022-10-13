#pragma once

#include <memory>
#include <vector>

template<typename T> struct StdTypeDefs {
	using Ptr = std::shared_ptr<T>;
	using CPtr = std::shared_ptr<const T>;
	using WPtr = std::weak_ptr<const T>;
	using Collection = std::vector<Ptr>;
	using CCollection = std::vector<CPtr>;
	using WCollection = std::vector<WPtr>;
};

template<typename T> struct ConstTypeDefs {
	using Ptr = std::shared_ptr<const T>;
	using Collection = std::vector<Ptr>;
};
