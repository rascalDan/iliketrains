#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <special_members.h>
#include <type_traits>
#include <vector>

template<typename Ptr, typename... Others> class Collection {
public:
	Collection() = default;
	virtual ~Collection() = default;

	DEFAULT_MOVE_NO_COPY(Collection);

	using Object = Ptr::element_type;
	using Objects = std::vector<Ptr>;
	template<typename T> using OtherObjects = std::vector<T *>;

	Collection &
	operator=(Objects && other)
	{
		objects = std::move(other);
		((std::get<OtherObjects<Others>>(otherObjects).clear()), ...);
		for (const auto & other : objects) {
			addOthersPtr(other.get());
		}
		return *this;
	}

	const Ptr &
	operator[](size_t idx) const
	{
		return objects[idx];
	}

	template<typename T = Object>
		requires(std::is_same_v<T, Object> || (std::is_base_of_v<Others, T> || ...))
	[[nodiscard]] auto
	size() const noexcept
	{
		return containerFor<T>().size();
	}

	template<typename T = Object, typename... Params>
	auto
	create(Params &&... params)
		requires std::is_base_of_v<Object, T>
	{
		if constexpr (requires(Ptr ptr) { ptr = std::make_shared<T>(std::forward<Params>(params)...); }) {
			auto obj = std::make_shared<T>(std::forward<Params>(params)...);
			objects.emplace_back(obj);
			addOthersType<T>(obj.get());
			return obj;
		}
		else {
			auto obj = static_cast<T *>(
					objects.emplace_back(std::make_unique<T>(std::forward<Params>(params)...)).get());
			addOthersType<T>(obj);
			return obj;
		}
	}

	template<typename T = Object>
	T *
	find()
	{
		const auto & srcObjects = containerFor<T>();
		if constexpr (std::is_convertible_v<typename std::remove_reference_t<decltype(srcObjects)>::value_type, T *>) {
			if (srcObjects.empty()) {
				return nullptr;
			}
			return srcObjects.front();
		}
		else if (auto i = std::find_if(srcObjects.begin(), srcObjects.end(),
						 [](auto && o) {
							 return dynamic_cast<T *>(std::to_address(o)) != nullptr;
						 });
				i != srcObjects.end()) {
			return static_cast<T *>(std::to_address(*i));
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
		const auto & srcObjects = containerFor<T>();
		return apply_internal<T>(srcObjects.begin(), srcObjects.end(), m, std::forward<Params>(params)...);
	}

	template<typename T = Object, typename... Params>
	auto
	rapply(const auto & m, Params &&... params) const
	{
		const auto & srcObjects = containerFor<T>();
		return apply_internal<T>(srcObjects.rbegin(), srcObjects.rend(), m, std::forward<Params>(params)...);
	}

	template<typename T = Object, typename... Params>
	auto
	applyOne(const auto & m, Params &&... params) const
	{
		const auto & srcObjects = containerFor<T>();
		return applyOne_internal<T>(srcObjects.begin(), srcObjects.end(), m, std::forward<Params>(params)...);
	}

	template<typename T = Object, typename... Params>
	auto
	rapplyOne(const auto & m, Params &&... params) const
	{
		const auto & srcObjects = containerFor<T>();
		return applyOne_internal<T>(srcObjects.rbegin(), srcObjects.rend(), m, std::forward<Params>(params)...);
	}

	template<typename T>
		requires std::is_base_of_v<Object, T>
	auto
	removeAll()
	{
		auto removeAllFrom = [](auto & container) {
			if constexpr (std::is_base_of_v<T, decltype(std::to_address(container.front()))>) {
				const auto size = container.size();
				container.clear();
				return size;
			}
			else {
				return std::erase_if(container, [](auto && objPtr) -> bool {
					return dynamic_cast<const T *>(std::to_address(objPtr));
				});
			}
		};
		(removeAllFrom(std::get<OtherObjects<Others>>(otherObjects)), ...);
		return removeAllFrom(objects);
	}

	void
	clear()
	{
		((std::get<OtherObjects<Others>>(otherObjects).clear()), ...);
		objects.clear();
	}

	[[nodiscard]] auto
	begin() const
	{
		return objects.begin();
	}

	[[nodiscard]] auto
	end() const
	{
		return objects.end();
	}

	[[nodiscard]] auto
	rbegin() const
	{
		return objects.rbegin();
	}

	[[nodiscard]] auto
	rend() const
	{
		return objects.rend();
	}

	[[nodiscard]] bool
	empty() const
	{
		return objects.empty();
	}

	decltype(auto)
	emplace(Ptr && ptr)
	{
		const auto & object = objects.emplace_back(std::move(ptr));
		addOthersPtr(object.get());
		return object;
	}

protected:
	Objects objects;
	std::tuple<OtherObjects<Others>...> otherObjects;

	template<typename T>
	void
	addOthersType(T * obj)
	{
		applyToOthersType<T>(
				[](auto & others, auto ptr) {
					others.emplace_back(ptr);
				},
				obj);
	}

	void
	addOthersPtr(Object * obj)
	{
		applyToOthersPtr(
				[](auto & others, auto ptr) {
					others.emplace_back(ptr);
				},
				obj);
	}

	template<typename T, typename... Params>
		requires(sizeof...(Others) == 0)
	void
	applyToOthersType(const auto &, Params...)
	{
	}

	void
	applyToOthersPtr(const auto &, Object *)
		requires(sizeof...(Others) == 0)
	{
	}

	template<typename T, typename... Params>
		requires(sizeof...(Others) > 0)
	void
	applyToOthersType(const auto & func, Params &&... params)
	{
		(
				[&]() {
					if constexpr (std::is_convertible_v<T *, Others *>) {
						std::invoke(
								func, std::get<OtherObjects<Others>>(otherObjects), std::forward<Params>(params)...);
					}
				}(),
				...);
	}

	void
	applyToOthersPtr(const auto & func, Object * obj)
		requires(sizeof...(Others) > 0)
	{
		(
				[&]() {
					if (auto ptr = dynamic_cast<Others *>(obj)) {
						std::invoke(func, std::get<OtherObjects<Others>>(otherObjects), ptr);
					}
				}(),
				...);
	}

	template<typename T>
		requires((std::is_base_of_v<Others, T> || ...))
	[[nodiscard]] consteval static size_t
	idx()
	{
		size_t typeIdx = 0;
		auto found = ((++typeIdx && std::is_base_of_v<Others, T>) || ...);
		return typeIdx - found;
	}

	template<typename T>
	[[nodiscard]]
	constexpr const auto &
	containerFor() const
	{
		if constexpr ((std::is_base_of_v<Others, T> || ...)) {
			return std::get<idx<T>()>(otherObjects);
		}
		else {
			return objects;
		}
	}

	template<typename T = Object, typename... Params>
	auto
	apply_internal(const auto begin, const auto end, const auto & m, Params &&... params) const
	{
		if constexpr (std::is_convertible_v<decltype(std::to_address(*begin)), T *>) {
			std::for_each(begin, end, [&m, &params...](auto && op) {
				std::invoke(m, op, std::forward<Params>(params)...);
			});
			return std::distance(begin, end);
		}
		else {
			return std::count_if(begin, end, [&m, &params...](auto && op) {
				if (auto o = dynamic_cast<T *>(std::to_address(op))) {
					std::invoke(m, o, std::forward<Params>(params)...);
					return true;
				}
				return false;
			});
		}
	}

	template<typename T = Object, typename... Params>
	auto
	applyOne_internal(const auto begin, const auto end, const auto & m, Params &&... params) const
	{
		if constexpr (std::is_convertible_v<decltype(std::to_address(*begin)), T *>) {
			return std::find_if(begin, end, [&m, &params...](auto && op) {
				return std::invoke(m, op, std::forward<Params>(params)...);
			});
		}
		else {
			return std::find_if(begin, end, [&m, &params...](auto && op) {
				if (auto o = dynamic_cast<T *>(std::to_address(op))) {
					return std::invoke(m, o, std::forward<Params>(params)...);
				}
				return false;
			});
		}
	}
};

template<typename T, typename... Others> using SharedCollection = Collection<std::shared_ptr<T>, Others...>;
template<typename T, typename... Others> using UniqueCollection = Collection<std::unique_ptr<T>, Others...>;
