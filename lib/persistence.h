#pragma once

#include "manyPtr.h"
#include <charconv>
#include <format>
#include <functional>
#include <glm/glm.hpp>
#include <iosfwd>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <special_members.h>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

namespace glm {
	template<glm::length_t L, typename T, glm::qualifier Q> struct vec;
}

namespace Persistence {
	struct Selection;
	using SelectionPtr = std::unique_ptr<Selection>;
	using Stack = std::stack<SelectionPtr>;

	struct Writer {
		Writer() = default;
		virtual ~Writer() = default;
		DEFAULT_MOVE_COPY(Writer);

		virtual void beginObject() const = 0;
		virtual void beginArray() const = 0;
		virtual void pushValue(bool value) const = 0;
		virtual void pushValue(float value) const = 0;
		virtual void pushValue(int value) const = 0;
		virtual void pushValue(std::nullptr_t) const = 0;
		virtual void pushValue(std::string_view value) const = 0;
		virtual void nextValue() const = 0;
		virtual void pushKey(std::string_view k) const = 0;
		virtual void endArray() const = 0;
		virtual void endObject() const = 0;
	};

	struct Selection {
		Selection() = default;
		virtual ~Selection() = default;
		DEFAULT_MOVE_COPY(Selection);

		virtual void setValue(std::string_view);
		virtual void setValue(bool);
		virtual void setValue(std::nullptr_t);
		virtual void setValue(std::string &&);
		virtual void beginArray(Stack &);
		virtual void beginObject(Stack &);
		virtual void endObject(Stack &);
		virtual void beforeValue(Stack &);
		[[nodiscard]] virtual SelectionPtr select(const std::string &);

		[[nodiscard]] virtual bool needsWrite() const;
		virtual void write(const Writer &) const;
	};

	template<typename T> struct SelectionT;

	template<typename T> struct SelectionV : public Selection {
		explicit SelectionV(T & value) : v {value} { }

		void
		beforeValue(Stack &) override
		{
		}

		[[nodiscard]] static SelectionPtr
		make(T & value)
		{
			return make_s<SelectionT<T>>(value);
		}

		template<typename S, typename... Extra>
		[[nodiscard]] static SelectionPtr
		make_s(T & value, Extra &&... extra)
		{
			return std::make_unique<S>(value, std::forward<Extra>(extra)...);
		}

		T & v;
	};

	template<typename T>
	concept Arithmatic = std::is_arithmetic_v<T>;

	template<> struct SelectionT<bool> : public SelectionV<bool> {
		using SelectionV<bool>::SelectionV;
		using Selection::setValue;

		void
		setValue(bool evalue) override
		{
			this->v = evalue;
		}

		void
		setValue(std::string && evalue) override
		{
			using namespace std::literals;
			if (!(this->v = evalue == "true"sv)) {
				if (evalue != "false"sv) {
					throw std::runtime_error("Value conversion failure");
				}
			}
		}

		void
		write(const Writer & out) const override
		{
			out.pushValue(this->v);
		}
	};

	template<Arithmatic T> struct SelectionT<T> : public SelectionV<T> {
		using SelectionV<T>::SelectionV;
		using Selection::setValue;

		void
		setValue(std::string_view evalue) override
		{
			if (auto res = std::from_chars(evalue.data(), evalue.data() + evalue.length(), this->v).ec;
					res != std::errc {}) {
				throw std::runtime_error("Value conversion failure");
			}
		}

		void
		setValue(std::string && evalue) override
		{
			setValue(std::string_view {evalue});
		}

		void
		write(const Writer & out) const override
		{
			out.pushValue(this->v);
		}
	};

	template<typename T> struct SelectionT : public SelectionV<T> {
		using SelectionV<T>::SelectionV;
		using Selection::setValue;

		void
		setValue(T && evalue) override
		{
			std::swap(this->v, evalue);
		}

		void
		write(const Writer & out) const override
		{
			out.pushValue(this->v);
		}
	};

	template<typename T> struct SelectionT<std::optional<T>> : public SelectionT<T> {
		explicit SelectionT(std::optional<T> & value) : SelectionT<T> {value.emplace()} { }
	};

	struct Persistable;

	struct PersistenceStore {
		using SelectionFactory = std::function<SelectionPtr()>;
		PersistenceStore() = default;
		virtual ~PersistenceStore() = default;
		DEFAULT_MOVE_NO_COPY(PersistenceStore);

		template<typename T> [[nodiscard]] inline bool persistType(const T * const, const std::type_info & ti);

		enum class NameAction { Push, HandleAndContinue, Ignore };
		using NameActionSelection = std::pair<NameAction, SelectionPtr>;

		template<typename Helper, typename T>
		[[nodiscard]] inline bool
		persistValue(const std::string_view key, T & value)
		{
			auto [act, s] = setName(key, [&value]() {
				return std::make_unique<Helper>(value);
			});
			if (act != NameAction::Ignore) {
				sel = std::move(s);
				if (act == NameAction::HandleAndContinue) {
					selHandler();
				}
			}
			return (act != NameAction::Push);
		}

		[[nodiscard]] virtual NameActionSelection setName(const std::string_view key, SelectionFactory &&) = 0;
		virtual void selHandler() { };
		virtual void setType(const std::string_view, const Persistable *) = 0;

		SelectionPtr sel {};
	};

	struct PersistenceSelect : public PersistenceStore {
		explicit PersistenceSelect(const std::string & n);

		NameActionSelection setName(const std::string_view key, SelectionFactory &&) override;

		void setType(const std::string_view, const Persistable *) override;

		const std::string & name;
	};

	struct PersistenceWrite : public PersistenceStore {
		explicit PersistenceWrite(const Writer & o, bool sh);

		NameActionSelection setName(const std::string_view key, SelectionFactory &&) override;

		void selHandler() override;

		void setType(const std::string_view tn, const Persistable * p) override;

		bool first {true};
		const Writer & out;
		bool shared;
	};

	template<typename T> struct SelectionT<std::span<T>> : public SelectionV<std::span<T>> {
		using V = std::span<T>;

		struct Members : public SelectionV<V> {
			using SelectionV<V>::SelectionV;

			void
			beforeValue(Stack & stk) override
			{
				stk.push(SelectionV<T>::make(this->v[idx++]));
			}

			std::size_t idx {0};

			void
			write(const Writer & out) const override
			{
				for (std::size_t n = 0; n < this->v.size(); n += 1) {
					if (n) {
						out.nextValue();
					}
					SelectionT<T> {this->v[n]}.write(out);
				}
			}
		};

		using SelectionV<V>::SelectionV;
		using SelectionV<V>::setValue;

		void
		setValue(std::string && s) override
		{
			std::stringstream ss {std::move(s)};
			for (std::size_t n = 0; n < this->v.size(); n += 1) {
				ss >> this->v[n];
				ss.get();
			}
		}

		void
		beginArray(Stack & stk) override
		{
			stk.push(this->template make_s<Members>(this->v));
		}

		void
		write(const Writer & out) const override
		{
			out.beginArray();
			Members {this->v}.write(out);
			out.endArray();
		}
	};

	template<glm::length_t L, typename T, glm::qualifier Q>
	struct SelectionT<glm::vec<L, T, Q>> : public SelectionT<std::span<T>> {
		SelectionT(glm::vec<L, T, Q> & v) : SelectionT<std::span<T>> {spn}, spn {&v[0], L} { }

		std::span<T> spn;
	};

	template<typename T> struct SelectionT<std::vector<T>> : public SelectionV<std::vector<T>> {
		using V = std::vector<T>;

		struct Members : public SelectionV<V> {
			using SelectionV<V>::SelectionV;

			void
			beforeValue(Stack & stk) override
			{
				stk.push(SelectionV<T>::make(this->v.emplace_back()));
			}

			void
			write(const Writer & out) const override
			{
				for (auto & member : this->v) {
					if (&member != &this->v.front()) {
						out.nextValue();
					}
					SelectionT<T> {member}.write(out);
				}
			}
		};

		using SelectionV<V>::SelectionV;

		void
		beginArray(Stack & stk) override
		{
			stk.push(this->template make_s<Members>(this->v));
		}

		void
		write(const Writer & out) const override
		{
			out.beginArray();
			Members {this->v}.write(out);
			out.endArray();
		}
	};

	template<typename... T> struct SelectionT<std::tuple<T...>> : public SelectionV<std::tuple<T...>> {
		using V = std::tuple<T...>;
		using SelectionV<V>::SelectionV;

		struct Members : public SelectionV<V> {
			template<size_t... Idx>
			explicit Members(V & v, std::integer_sequence<size_t, Idx...>) :
				SelectionV<V> {v}, members {SelectionV<std::tuple_element_t<Idx, V>>::make(std::get<Idx>(v))...}
			{
			}

			void
			beforeValue(Stack & stk) override
			{
				stk.push(std::move(members[idx++]));
			}

			std::size_t idx {0};
			std::array<SelectionPtr, std::tuple_size_v<V>> members;
		};

		void
		beginArray(Stack & stk) override
		{
			stk.push(this->template make_s<Members>(
					this->v, std::make_integer_sequence<size_t, std::tuple_size_v<V>>()));
		}
	};

	template<typename T, typename U> struct SelectionT<std::pair<T, U>> : public SelectionV<std::pair<T, U>> {
		using V = std::pair<T, U>;
		using SelectionV<V>::SelectionV;

		struct Members : public SelectionV<V> {
			explicit Members(V & v) :
				SelectionV<V> {v}, members {
										   SelectionV<T>::make(v.first),
										   SelectionV<U>::make(v.second),
								   }
			{
			}

			void
			beforeValue(Stack & stk) override
			{
				stk.push(std::move(members[idx++]));
			}

			std::size_t idx {0};
			std::array<SelectionPtr, 2> members;
		};

		void
		beginArray(Stack & stk) override
		{
			stk.push(this->template make_s<Members>(this->v));
		}
	};

	template<typename Map, typename Type = typename Map::mapped_type, auto Key = &Type::element_type::id>
	struct MapByMember : public Persistence::SelectionT<Type> {
		MapByMember(Map & m) : Persistence::SelectionT<Type> {s}, map {m} { }

		using Persistence::SelectionT<Type>::SelectionT;

		void
		endObject(Persistence::Stack & stk) override
		{
			// TODO test with unique_ptr
			map.emplace(std::invoke(Key, s), std::move(s));
			Persistence::SelectionT<Type>::endObject(stk);
		}

	private:
		Type s;
		Map & map;
	};

	template<typename Container, typename Type = typename Container::value_type>
	struct Appender : public Persistence::SelectionT<Type> {
		Appender(Container & c) : Persistence::SelectionT<Type> {s}, container {c} { }

		using Persistence::SelectionT<Type>::SelectionT;

		void
		endObject(Persistence::Stack & stk) override
		{
			// TODO test with unique_ptr
			container.emplace_back(std::move(s));
			Persistence::SelectionT<Type>::endObject(stk);
		}

	private:
		Type s;
		Container & container;
	};

	struct Persistable {
		Persistable() = default;
		virtual ~Persistable() = default;
		DEFAULT_MOVE_COPY(Persistable);

		virtual bool persist(PersistenceStore & store) = 0;
		virtual void postLoad();

		[[nodiscard]] virtual std::string getId() const;

		template<typename T>
		[[nodiscard]] constexpr static auto
		typeName()
		{
			constexpr std::string_view name {__PRETTY_FUNCTION__};
			constexpr auto s {name.find("T = ") + 4}, e {name.rfind(']')};
			return name.substr(s, e - s);
		}

		template<typename T> static void addFactory() __attribute__((constructor));
		static void addFactory(const std::string_view, std::function<std::unique_ptr<Persistable>()>,
				std::function<std::shared_ptr<Persistable>()>);
		[[nodiscard]] static std::unique_ptr<Persistable> callFactory(const std::string_view);
		[[nodiscard]] static std::shared_ptr<Persistable> callSharedFactory(const std::string_view);
	};

	template<typename T>
	void
	Persistable::addFactory()
	{
		addFactory(typeName<T>(), std::make_unique<T>, std::make_shared<T>);
	}

	template<typename T>
	inline bool
	PersistenceStore::persistType(const T * const v, const std::type_info & ti)
	{
		if constexpr (!std::is_abstract_v<T>) {
			[[maybe_unused]] constexpr auto f = &Persistable::addFactory<T>;
		}
		if (typeid(std::decay_t<T>) == ti) {
			setType(Persistable::typeName<T>(), v);
		}
		return true;
	}

	class ParseBase {
	public:
		using SharedObjects = std::map<std::string, std::shared_ptr<Persistable>>;
		using SharedObjectsWPtr = std::weak_ptr<SharedObjects>;
		using SharedObjectsPtr = std::shared_ptr<SharedObjects>;

		ParseBase();
		DEFAULT_MOVE_NO_COPY(ParseBase);

		template<typename T>
		static auto
		getShared(auto && k)
		{
			return std::dynamic_pointer_cast<T>(Persistence::ParseBase::sharedObjects.lock()->at(k));
		}

		template<typename... T>
		static auto
		emplaceShared(T &&... v)
		{
			return sharedObjects.lock()->emplace(std::forward<T>(v)...);
		}

	protected:
		Stack stk;

	private:
		inline static thread_local SharedObjectsWPtr sharedObjects;
		SharedObjectsPtr sharedObjectsInstance;
	};

	// TODO Move these
	using SeenSharedObjects = std::map<void *, std::string>;
	inline SeenSharedObjects seenSharedObjects;

	template<typename Ptr> struct SelectionPtrBase : public SelectionV<Ptr> {
		static constexpr auto shared = std::is_copy_assignable_v<Ptr>;
		using T = typename Ptr::element_type;

		struct SelectionObj : public SelectionV<Ptr> {
			struct MakeObjectByTypeName : public SelectionV<Ptr> {
				using SelectionV<Ptr>::SelectionV;
				using Selection::setValue;

				void
				setValue(std::string && type) override
				{
					if constexpr (shared) {
						auto no = Persistable::callSharedFactory(type);
						if (auto tno = std::dynamic_pointer_cast<T>(no)) {
							this->v = std::move(tno);
							return;
						}
					}
					else {
						auto no = Persistable::callFactory(type);
						if (dynamic_cast<T *>(no.get())) {
							this->v.reset(static_cast<T *>(no.release()));
							return;
						}
					}
					throw std::runtime_error("Named type doesn't cast to target type");
				}
			};

			struct RememberObjectById : public SelectionV<Ptr> {
				using SelectionV<Ptr>::SelectionV;
				using Selection::setValue;

				void
				setValue(std::string && id) override
				{
					ParseBase::emplaceShared(id, this->v);
				}
			};

			using SelectionV<Ptr>::SelectionV;

			[[nodiscard]] SelectionPtr
			select(const std::string & mbr) override
			{
				using namespace std::literals;
				if (mbr == "p.typeid"sv) {
					if (this->v) {
						throw std::runtime_error("cannot set object type after creation");
					}
					return this->template make_s<MakeObjectByTypeName>(this->v);
				}
				if constexpr (shared) {
					if (mbr == "p.id"sv) {
						make_default_as_needed(this->v);
						return this->template make_s<RememberObjectById>(this->v);
					}
				}
				make_default_as_needed(this->v);
				PersistenceSelect ps {mbr};
				if (this->v->persist(ps)) {
					throw std::runtime_error {std::format("Cannot find member: {}", mbr)};
				}
				return std::move(ps.sel);
			}

			void
			endObject(Stack & stk) override
			{
				make_default_as_needed(this->v);
				if (this->v) {
					this->v->postLoad();
				}
				stk.pop();
			}

			void
			write(const Writer & out) const override
			{
				out.beginObject();
				PersistenceWrite pw {out, shared};
				this->v->persist(pw);
				out.endObject();
			}
		};

		static inline void
		make_default_as_needed(Ptr & v)
		{
			if (!v) {
				if constexpr (std::is_abstract_v<T>) {
					throw std::runtime_error("cannot select member of null abstract object");
				}
				else if constexpr (shared) {
					v = std::make_shared<T>();
				}
				else {
					v = std::make_unique<T>();
				}
			}
		}

		using SelectionV<Ptr>::SelectionV;
		using Selection::setValue;

		void
		setValue(std::nullptr_t) override
		{
			this->v.reset();
		}

		void
		beginObject(Stack & stk) override
		{
			stk.push(this->template make_s<SelectionObj>(this->v));
		}

		void
		endObject(Stack & stk) override
		{
			stk.pop();
		}

		[[nodiscard]] bool
		needsWrite() const override
		{
			return this->v != nullptr;
		}

		void
		write(const Writer & out) const override
		{
			if (this->v) {
				if constexpr (shared) {
					if (const auto existing = seenSharedObjects.find(std::to_address(this->v));
							existing != seenSharedObjects.end()) {
						out.pushValue(existing->second);
						return;
					}
					seenSharedObjects.emplace(std::to_address(this->v), this->v->getId());
				}
				SelectionObj {this->v}.write(out);
			}
			else {
				out.pushValue(nullptr);
			}
		}
	};

	template<typename T> struct SelectionT<std::unique_ptr<T>> : public SelectionPtrBase<std::unique_ptr<T>> {
		using SelectionPtrBase<std::unique_ptr<T>>::SelectionPtrBase;
	};

	template<typename T> struct SelectionT<std::shared_ptr<T>> : public SelectionPtrBase<std::shared_ptr<T>> {
		using SelectionPtrBase<std::shared_ptr<T>>::SelectionPtrBase;
		using SelectionPtrBase<std::shared_ptr<T>>::setValue;

		void
		setValue(std::string && id) override
		{
			if (auto teo = ParseBase::getShared<T>(id)) {
				this->v = std::move(teo);
			}
			else {
				throw std::runtime_error("Named type doesn't cast to target type");
			}
		}
	};
}

#define STORE_TYPE store.persistType(this, typeid(*this))
#define STORE_MEMBER(mbr) STORE_NAME_MEMBER(#mbr, mbr)
#define STORE_NAME_MEMBER(name, mbr) store.persistValue<Persistence::SelectionT<decltype(mbr)>>(name, mbr)
#define STORE_HELPER(mbr, Helper) STORE_NAME_HELPER(#mbr, mbr, Helper)
#define STORE_NAME_HELPER(name, mbr, Helper) store.persistValue<Helper>(name, mbr)
