#ifndef CACHE_H
#define CACHE_H

#include <map>
#include <memory>

template<typename Obj> class Cache {
public:
	using Ptr = std::shared_ptr<Obj>;

	virtual ~Cache() = default;

	[[nodiscard]] Ptr
	get(const std::string & key)
	{
		if (auto e = cached.find(key); e != cached.end()) {
			return e->second;
		}
		return cached.emplace(key, construct(key)).first->second;
	}

	[[nodiscard]] virtual Ptr
	construct(const std::string & key) const
	{
		return std::make_shared<Obj>(key);
	}

private:
	std::map<std::string, Ptr> cached;
};

#endif
