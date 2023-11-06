#pragma once

#include <memory>
#include <special_members.h>

template<typename Obj, auto Destroy> class wrapped_ptr {
public:
	template<typename... Args, typename... Params>
	explicit wrapped_ptr(Obj * (*factory)(Params...), Args &&... args) : obj {factory(std::forward<Args>(args)...)}
	{
	}

	explicit wrapped_ptr(wrapped_ptr && p) : obj {p.obj}
	{
		p.obj = nullptr;
	}

	~wrapped_ptr()
	{
		if (obj) {
			Destroy(obj);
		}
	}

	NO_COPY(wrapped_ptr);

	wrapped_ptr &
	operator=(wrapped_ptr && p)
	{
		if (obj) {
			Destroy(obj);
		}
		obj = p.obj;
		p.obj = nullptr;
		return *this;
	}

	[[nodiscard]] inline
	operator Obj *() const noexcept
	{
		return obj;
	}

	[[nodiscard]] inline auto
	operator->() const noexcept
	{
		return obj;
	}

	[[nodiscard]] inline auto
	get() const noexcept
	{
		return obj;
	}

protected:
	explicit wrapped_ptr(Obj * o) : obj {o} { }

	Obj * obj;
};

template<typename Obj, auto Create, auto Destroy> class wrapped_ptrt : public wrapped_ptr<Obj, Destroy> {
public:
	template<typename... Args>
	explicit wrapped_ptrt(Args &&... args) : wrapped_ptr<Obj, Destroy> {Create(std::forward<Args>(args)...)}
	{
	}
};
