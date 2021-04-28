#include "persistance.h"
#include <map>

namespace Persistanace {
	using NamedTypeFactories = std::map<std::string_view, std::function<std::unique_ptr<Persistable>()>>;
	static NamedTypeFactories namedTypeFactories;

	void
	Persistable::addFactory(const std::string_view t, std::function<std::unique_ptr<Persistable>()> f)
	{
		namedTypeFactories.emplace(t, std::move(f));
	}

	std::unique_ptr<Persistable>
	Persistable::callFactory(const std::string_view t)
	{
		return namedTypeFactories.at(t)();
	}

	void
	Selection::setValue(float &)
	{
		throw std::runtime_error("Unexpected float");
	}

	void
	Selection::setValue(bool &)
	{
		throw std::runtime_error("Unexpected bool");
	}

	void
	Selection::setValue(const std::nullptr_t &)
	{
		throw std::runtime_error("Unexpected null");
	}

	void
	Selection::setValue(std::string &)
	{
		throw std::runtime_error("Unexpected string");
	}

	void
	Selection::beginArray(Stack &)
	{
		throw std::runtime_error("Unexpected array");
	}

	void
	Selection::beginObject(Stack &)
	{
		throw std::runtime_error("Unexpected object");
	}

	void
	Selection::beforeValue(Stack &)
	{
		throw std::runtime_error("Unexpected value");
	}

	SelectionPtr
	Selection::select(const std::string &)
	{
		throw std::runtime_error("Unexpected select");
	}

	void
	Selection::endObject(Stack &)
	{
	}
}
