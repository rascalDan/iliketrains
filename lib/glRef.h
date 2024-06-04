#pragma once

#include <special_members.h>
#include <stdexcept>
#include <utility>

template<typename IdType, auto get, auto release, auto... fixed> class glRef {
public:
	// cppcheck-suppress redundantPointerOp
	template<typename... Args> explicit glRef(Args &&... args) : id {(*get)(fixed..., std::forward<Args>(args)...)}
	{
		if (!id) {
			throw std::runtime_error("Get function failed");
		}
	}

	glRef(glRef && other) noexcept : id {std::exchange(other.id, {})} { }

	~glRef()
	{
		if (id) {
			// cppcheck-suppress redundantPointerOp
			(*release)(id);
		}
	}

	NO_COPY(glRef);

	auto &
	operator=(glRef && other) noexcept
	{
		if (id) {
			// cppcheck-suppress redundantPointerOp
			(*release)(id);
		}
		id = std::exchange(other.id, {});
		return *this;
	}

	auto
	operator*() const
	{
		return id;
	}

	auto
	operator->() const
	{
		return id;
	}

	operator IdType() const
	{
		return id;
	}

private:
	IdType id {};
};
