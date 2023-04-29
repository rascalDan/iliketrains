#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <type_traits>
#include <vector>

template<typename Object, bool shared = true> class Collection {
public:
	virtual ~Collection() = default;

	using Ptr = std::conditional_t<shared, std::shared_ptr<Object>, std::unique_ptr<Object>>;
	using Objects = std::vector<Ptr>;
	Objects objects;

	template<typename T = Object, typename... Params>
	auto
	create(Params &&... params)
		requires std::is_base_of_v<Object, T>
	{
		if constexpr (shared) {
			auto obj = std::make_shared<T>(std::forward<Params>(params)...);
			objects.emplace_back(obj);
			return obj;
		}
		else {
			return static_cast<T *>(objects.emplace_back(std::make_unique<T>(std::forward<Params>(params)...)).get());
		}
	}

	template<typename T = Object>
	T *
	find()
	{
		if (auto i = std::find_if(objects.begin(), objects.end(),
					[](auto && o) {
						return (dynamic_cast<T *>(o.get()));
					});
				i != objects.end()) {
			return static_cast<T *>(i->get());
		}
		return nullptr;
	}

	template<typename T = Object, typename... Params>
	auto
	findOrCreate(Params &&... params)
		requires std::is_base_of_v<Object, T>
	{
		if (auto o = find<T>()) {
			return o;
		}
		return create<T>(std::forward<Params>(params)...).get();
	}

	template<typename T = Object, typename... Params>
	auto
	apply(const auto & m, Params &&... params) const
	{
		return apply_internal<T>(objects.begin(), objects.end(), m, std::forward<Params>(params)...);
	}

	template<typename T = Object, typename... Params>
	auto
	rapply(const auto & m, Params &&... params) const
	{
		return apply_internal<T>(objects.rbegin(), objects.rend(), m, std::forward<Params>(params)...);
	}

	template<typename T = Object, typename... Params>
	auto
	applyOne(const auto & m, Params &&... params) const
	{
		return applyOne_internal<T>(objects.begin(), objects.end(), m, std::forward<Params>(params)...);
	}

	template<typename T = Object, typename... Params>
	auto
	rapplyOne(const auto & m, Params &&... params) const
	{
		return applyOne_internal<T>(objects.rbegin(), objects.rend(), m, std::forward<Params>(params)...);
	}

	template<typename T = Object>
	auto
	removeAll()
	{
		return std::erase_if(objects, [](auto && op) {
			return dynamic_cast<T *>(op.get());
		});
	}

	auto
	end() const
	{
		return objects.end();
	}

	auto
	rend() const
	{
		return objects.rend();
	}

protected:
	template<typename T = Object, typename... Params>
	auto
	apply_internal(const auto begin, const auto end, const auto & m, Params &&... params) const
	{
		return std::count_if(begin, end, [&m, &params...](auto && op) {
			if (auto o = dynamic_cast<T *>(op.get())) {
				std::invoke(m, o, std::forward<Params>(params)...);
				return true;
			}
			return false;
		});
	}

	template<typename T = Object, typename... Params>
	auto
	applyOne_internal(const auto begin, const auto end, const auto & m, Params &&... params) const
	{
		return std::find_if(begin, end, [&m, &params...](auto && op) {
			if (auto o = dynamic_cast<T *>(op.get())) {
				return std::invoke(m, o, std::forward<Params>(params)...);
			}
			return false;
		});
	}
};
