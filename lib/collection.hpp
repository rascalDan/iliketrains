#ifndef COLLECTION_H
#define COLLECTION_H

#include <algorithm>
#include <memory>
#include <type_traits>

template<typename Object, bool shared = true> class Collection {
public:
	virtual ~Collection() = default;

	using Ptr = std::conditional_t<shared, std::shared_ptr<Object>, std::unique_ptr<Object>>;
	using Objects = std::vector<Ptr>;
	Objects objects;

	template<typename T = Object, typename... Params>
	auto
	create(Params &&... params) requires std::is_base_of_v<Object, T>
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

	template<typename T = Object, typename M = void, typename... Params>
	auto
	apply(const M & m, Params &&... params) const
	{
		return std::count_if(objects.begin(), objects.end(), [&m, &params...](auto && op) {
			if (auto o = dynamic_cast<T *>(op.get())) {
				std::invoke(m, o, std::forward<Params>(params)...);
				return true;
			}
			return false;
		});
	}

	template<typename T = Object, typename M = void, typename... Params>
	auto
	applyOne(const M & m, Params &&... params) const
	{
		return std::find_if(objects.begin(), objects.end(), [&m, &params...](auto && op) {
			if (auto o = dynamic_cast<T *>(op.get())) {
				return std::invoke(m, o, std::forward<Params>(params)...);
			}
			return false;
		});
	}

	template<typename T = Object>
	void
	removeAll()
	{
		objects.erase(std::remove_if(objects.begin(), objects.end(),
							  [](auto && op) {
								  return dynamic_cast<T *>(op.get());
							  }),
				objects.end());
	}

	auto
	end() const
	{
		return objects.end();
	}
};

#endif
