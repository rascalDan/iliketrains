#include "persistence.h"
#include <map>
#include <sstream>

namespace Persistence {
	using Factories
			= std::pair<std::function<std::unique_ptr<Persistable>()>, std::function<std::shared_ptr<Persistable>()>>;
	using NamedTypeFactories = std::map<std::string_view, Factories>;

	inline static auto &
	namedTypeFactories()
	{
		static NamedTypeFactories namedTypeFactories;
		return namedTypeFactories;
	}

	void
	Persistable::addFactory(const std::string_view t, std::function<std::unique_ptr<Persistable>()> fu,
			std::function<std::shared_ptr<Persistable>()> fs)
	{
		namedTypeFactories().emplace(t, std::make_pair(std::move(fu), std::move(fs)));
	}

	std::unique_ptr<Persistable>
	Persistable::callFactory(const std::string_view t)
	{
		return namedTypeFactories().at(t).first();
	}

	std::shared_ptr<Persistable>
	Persistable::callSharedFactory(const std::string_view t)
	{
		return namedTypeFactories().at(t).second();
	}

	[[nodiscard]] std::string
	Persistable::getId() const
	{
		std::stringstream ss;
		ss << std::hex << this;
		return ss.str();
	}

	void
	Persistable::postLoad()
	{
	}

	PersistenceSelect::PersistenceSelect(const std::string & n) : name {n} { }

	PersistenceStore::NameActionSelection
	PersistenceSelect::setName(const std::string_view key, SelectionFactory && factory)
	{
		if (key == name) {
			return {NameAction::Push, factory()};
		}
		else {
			return {NameAction::Ignore, nullptr};
		}
	}

	void
	PersistenceSelect::setType(const std::string_view, const Persistable *)
	{
	}

	PersistenceWrite::PersistenceWrite(const Writer & o, bool sh) : out {o}, shared {sh} { }

	PersistenceStore::NameActionSelection
	PersistenceWrite::setName(const std::string_view key, SelectionFactory && factory)
	{
		auto s = factory();
		if (s->needsWrite()) {
			if (!first) {
				out.nextValue();
			}
			else {
				first = false;
			}
			out.pushKey(key);
			return {NameAction::HandleAndContinue, std::move(s)};
		}
		return {NameAction::Ignore, nullptr};
	}

	void
	PersistenceWrite::selHandler()
	{
		this->sel->write(out);
	}

	void
	PersistenceWrite::setType(const std::string_view tn, const Persistable * p)
	{
		out.pushKey("p.typeid");
		out.pushValue(tn);
		first = false;
		if (shared) {
			out.nextValue();
			out.pushKey("p.id");
			out.pushValue(p->getId());
		}
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

	void
	Selection::setValue(std::nullptr_t)
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

	bool
	Selection::needsWrite() const
	{
		return true;
	}

	void
	Selection::write(const Writer &) const
	{
		throw std::logic_error("Default write op shouldn't ever get called");
	}
	/// LCOV_EXCL_STOP

	ParseBase::ParseBase() : sharedObjectsInstance {std::make_shared<SharedObjects>()}
	{
		sharedObjects = sharedObjectsInstance;
	}
}
