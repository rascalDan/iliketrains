#include "persistence.h"
#include <map>

namespace Persistence {
	using Factories
			= std::pair<std::function<std::unique_ptr<Persistable>()>, std::function<std::shared_ptr<Persistable>()>>;
	using NamedTypeFactories = std::map<std::string_view, Factories>;
	static NamedTypeFactories namedTypeFactories;

	void
	Persistable::addFactory(const std::string_view t, std::function<std::unique_ptr<Persistable>()> fu,
			std::function<std::shared_ptr<Persistable>()> fs)
	{
		namedTypeFactories.emplace(t, std::make_pair(std::move(fu), std::move(fs)));
	}

	std::unique_ptr<Persistable>
	Persistable::callFactory(const std::string_view t)
	{
		return namedTypeFactories.at(t).first();
	}

	std::shared_ptr<Persistable>
	Persistable::callSharedFactory(const std::string_view t)
	{
		return namedTypeFactories.at(t).second();
	}

	void
	Selection::setValue(float)
	{
		throw std::runtime_error("Unexpected float");
	}

	void
	Selection::setValue(bool)
	{
		throw std::runtime_error("Unexpected bool");
	}

	void Selection::setValue(std::nullptr_t)
	{
		throw std::runtime_error("Unexpected null");
	}

	void
	Selection::setValue(std::string &&)
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

	/// LCOV_EXCL_START Don't think we can trigger these from something lexer will parse
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
	/// LCOV_EXCL_STOP
}
