#pragma once

#include "special_members.h"
#include <functional>
#include <map>
#include <memory>
#include <tuple>

template<typename Obj, typename... KeyParts> class Cache {
public:
	using Ptr = std::shared_ptr<Obj>;
	using Key = std::tuple<KeyParts...>;

	Cache() = default;
	virtual ~Cache() = default;
	DEFAULT_MOVE(Cache);
	NO_COPY(Cache);

	[[nodiscard]] Ptr
	get(const KeyParts &... keyparts)
	{
		auto key = std::tie(keyparts...);
		if (auto e = cached.find(key); e != cached.end()) {
			return e->second;
		}
		return cached.emplace(key, construct(keyparts...)).first->second;
	}

	[[nodiscard]] virtual Ptr
	construct(const KeyParts &... keyparts) const
	{
		return std::make_shared<Obj>(keyparts...);
	}

private:
	std::map<Key, Ptr, std::less<>> cached;
};

// IWYU pragma: no_forward_declare Cache
