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
	Selection::operator()(float &)
	{
		throw std::runtime_error("Unexpected float");
	}

	void
	Selection::operator()(bool &)
	{
		throw std::runtime_error("Unexpected bool");
	}

	void
	Selection::operator()(const std::nullptr_t &)
	{
		throw std::runtime_error("Unexpected null");
	}

	void
	Selection::operator()(std::string &)
	{
		throw std::runtime_error("Unexpected string");
	}

	void
	Selection::BeginArray(Stack &)
	{
		throw std::runtime_error("Unexpected array");
	}

	void
	Selection::BeginObject(Stack &)
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
	Selection::EndObject(Stack &)
	{
	}
}
