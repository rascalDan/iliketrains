#ifndef PTR_H
#define PTR_H

#include <memory>

template<typename Obj> class wrapped_ptr : public std::unique_ptr<Obj, void (*)(Obj *)> {
public:
	using std::unique_ptr<Obj, void (*)(Obj *)>::unique_ptr;
	wrapped_ptr() : std::unique_ptr<Obj, void (*)(Obj *)> {{}, {}} { }

	inline
	operator Obj *() const
	{
		return this->get();
	}

	template<typename... Args, typename... Params>
	static auto
	create(Obj * (*factory)(Args...), void (*deleter)(Obj *), Params &&... params)
	{
		return wrapped_ptr<Obj> {factory(std::forward<Params>(params)...), deleter};
	}
};

#endif
