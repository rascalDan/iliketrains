#pragma once

#include <memory>
#include <vector>

template<typename T> struct AnyPtr {
	// cppcheck-suppress noExplicitConstructor
	AnyPtr(T * p) : ptr {p} { }

	// cppcheck-suppress noExplicitConstructor
	template<typename S> AnyPtr(const S & p) : ptr {p.get()} { }

	auto
	get() const
	{
		return ptr;
	}

	auto
	operator->() const
	{
		return ptr;
	}

	auto &
	operator*() const
	{
		return *ptr;
	}

	// NOLINTNEXTLINE(hicpp-explicit-conversions)
	operator bool() const
	{
		return ptr != nullptr;
	}

	bool
	operator!() const
	{
		return ptr == nullptr;
	}

private:
	T * ptr;
};

template<typename T> struct StdTypeDefs {
	using AnyPtr = ::AnyPtr<T>;
	using AnyCPtr = ::AnyPtr<const T>;
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
