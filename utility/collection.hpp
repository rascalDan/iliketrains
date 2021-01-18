#ifndef COLLECTION_H
#define COLLECTION_H

#include <algorithm>
#include <memory>

template<typename Object> class Collection {
public:
	using Ptr = std::unique_ptr<Object>;
	std::vector<Ptr> objects;

	template<typename T = Object, typename... Params>
	const auto &
	create(Params &&... params)
	{
		return objects.emplace_back(std::make_unique<T>(std::forward<Params>(params)...));
	}

	template<typename T = Object, typename M = void, typename... Params>
	void
	apply(const M & m, Params &&... params) const
	{
		std::for_each(objects.cbegin(), objects.cend(), [&m, &params...](auto && op) {
			if (auto o = dynamic_cast<T *>(op.get())) {
				std::invoke(m, o, std::forward<Params>(params)...);
			}
		});
	}
};

#endif
