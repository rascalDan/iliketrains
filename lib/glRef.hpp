#ifndef GLREF_H
#define GLREF_H

#include <special_members.hpp>
#include <stdexcept>

template<typename IdType, auto & get, auto & release, auto... fixed> class glRef {
public:
	template<typename... Args> explicit glRef(Args &&... args) : id {get(fixed..., std::forward<Args>(args)...)}
	{
		if (!id) {
			throw std::runtime_error("Get function failed");
		}
	}

	glRef(glRef && other) : id {other.id}
	{
		other.id = {};
	}

	~glRef()
	{
		if (id) {
			release(id);
		}
	}

	NO_COPY(glRef);

	const auto &
	operator=(glRef && other)
	{
		if (id) {
			release(id);
		}
		id = other.id;
		other.id = {};
		return *this;
	}

	auto
	operator*() const
	{
		return id;
	}

	operator IdType() const
	{
		return id;
	}

private:
	IdType id;
};

#endif
