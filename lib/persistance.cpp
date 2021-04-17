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

	SelectionPtr
	Selection::BeginObject()
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
		throw std::runtime_error("Unexpected persist");
	}

	static_assert(!SelectionT<float>::ArrayLike);
	static_assert(!SelectionT<bool>::ArrayLike);
	static_assert(!SelectionT<std::string>::ArrayLike);
	static_assert(SelectionT<std::vector<float>>::ArrayLike);
	static_assert(SelectionT<glm::vec3>::ArrayLike);
	static_assert(SelectionT<std::vector<glm::vec3>>::ArrayLike);
}
