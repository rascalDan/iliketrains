#ifndef PERSISTANCE_H
#define PERSISTANCE_H

#include <functional>
#include <glm/glm.hpp>
#include <iosfwd>
#include <map>
#include <memory>
#include <special_members.hpp>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace glm {
	template<glm::length_t L, typename T, glm::qualifier Q> struct vec;
}

namespace Persistence {
	struct Selection;
	using SelectionPtr = std::unique_ptr<Selection>;
	using Stack = std::stack<SelectionPtr>;

	struct Selection {
		Selection() = default;
		virtual ~Selection() = default;
		DEFAULT_MOVE_COPY(Selection);

		virtual void setValue(float);
		virtual void setValue(bool);
		virtual void setValue(std::nullptr_t);
		virtual void setValue(std::string &&);
		virtual void beginArray(Stack &);
		virtual void beginObject(Stack &);
		virtual void endObject(Stack &);
		virtual void beforeValue(Stack &);
		virtual SelectionPtr select(const std::string &);
	};

	template<typename T> struct SelectionT;

	template<typename T> struct SelectionV : public Selection {
		explicit SelectionV(T & value) : v {value} { }

		void
		beforeValue(Stack &) override
		{
		}

		static SelectionPtr
		make(T & value)
		{
			return make_s<SelectionT<T>>(value);
		}

		template<typename S>
		static SelectionPtr
		make_s(T & value)
		{
			return std::make_unique<S>(value);
		}

		T & v;
	};

	template<typename T> struct SelectionT : public SelectionV<T> {
		using SelectionV<T>::SelectionV;
		using P = std::conditional_t<std::is_scalar_v<T>, T, T &&>;

		void
		setValue(P evalue) override
		{
			std::swap(this->v, evalue);
		}
	};

	struct PersistenceStore {
		template<typename T> inline bool persistType() const;

		template<typename T>
		inline bool
		persistValue(const std::string_view key, T & value)
		{
			if (key == name) {
				sel = SelectionV<T>::make(value);
				return false;
			}
			return true;
		}
		const std::string & name;
		SelectionPtr sel {};
	};

	template<glm::length_t L, typename T, glm::qualifier Q>
	struct SelectionT<glm::vec<L, T, Q>> : public SelectionV<glm::vec<L, T, Q>> {
		using V = glm::vec<L, T, Q>;

		struct Members : public SelectionV<V> {
			using SelectionV<V>::SelectionV;

			void
			beforeValue(Stack & stk) override
			{
				stk.push(SelectionV<T>::make(this->v[idx++]));
			}

			glm::length_t idx {0};
		};

		using SelectionV<V>::SelectionV;

		void
		beginArray(Stack & stk) override
		{
			stk.push(this->template make_s<Members>(this->v));
		}
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
		};

		using SelectionV<V>::SelectionV;

		void
		beginArray(Stack & stk) override
		{
			stk.push(this->template make_s<Members>(this->v));
		}
	};

	struct Persistable {
		Persistable() = default;
		virtual ~Persistable() = default;
		DEFAULT_MOVE_COPY(Persistable);

		virtual bool persist(PersistenceStore & store) = 0;

		template<typename T>
		constexpr static auto
		typeName()
		{
			constexpr std::string_view name {__PRETTY_FUNCTION__};
			constexpr auto s {name.find("T = ") + 4}, e {name.rfind(']')};
			return name.substr(s, e - s);
		}

		template<typename T> static void addFactory() __attribute__((constructor));
		static void addFactory(const std::string_view, std::function<std::unique_ptr<Persistable>()>,
				std::function<std::shared_ptr<Persistable>()>);
		static std::unique_ptr<Persistable> callFactory(const std::string_view);
		static std::shared_ptr<Persistable> callSharedFactory(const std::string_view);
	};

	template<typename T>
	void
	Persistable::addFactory()
	{
		addFactory(typeName<T>(), std::make_unique<T>, std::make_shared<T>);
	}

	template<typename T>
	inline bool
	PersistenceStore::persistType() const
	{
		if constexpr (!std::is_abstract_v<T>) {
			[[maybe_unused]] constexpr auto f = &Persistable::addFactory<T>;
		}
		return true;
	}

	// TODO Move this
	using SharedObjects = std::map<std::string, std::shared_ptr<Persistable>>;
	inline SharedObjects sharedObjects;

	template<typename Ptr, bool shared> struct SelectionPtrBase : public SelectionV<Ptr> {
		using T = typename Ptr::element_type;
		struct SelectionObj : public SelectionV<Ptr> {
			struct MakeObjectByTypeName : public SelectionV<Ptr> {
				using SelectionV<Ptr>::SelectionV;

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

				void
				setValue(std::string && id) override
				{
					sharedObjects.emplace(id, this->v);
				}
			};

			using SelectionV<Ptr>::SelectionV;

			SelectionPtr
			select(const std::string & mbr) override
			{
				using namespace std::literals;
				if (mbr == "@typeid"sv) {
					if (this->v) {
						throw std::runtime_error("cannot set object type after creation");
					}
					return this->template make_s<MakeObjectByTypeName>(this->v);
				}
				if constexpr (shared) {
					if (mbr == "@id"sv) {
						return this->template make_s<RememberObjectById>(this->v);
					}
				}
				make_default_as_needed(this->v);
				PersistenceStore ps {mbr};
				if (this->v->persist(ps)) {
					throw std::runtime_error("cannot find member: " + mbr);
				}
				return std::move(ps.sel);
			}

			void
			endObject(Stack & stk) override
			{
				make_default_as_needed(this->v);
				stk.pop();
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

		void setValue(std::nullptr_t) override
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
	};

	template<typename T> struct SelectionT<std::unique_ptr<T>> : public SelectionPtrBase<std::unique_ptr<T>, false> {
		using SelectionPtrBase<std::unique_ptr<T>, false>::SelectionPtrBase;
	};

	template<typename T> struct SelectionT<std::shared_ptr<T>> : public SelectionPtrBase<std::shared_ptr<T>, true> {
		using SelectionPtrBase<std::shared_ptr<T>, true>::SelectionPtrBase;

		void
		setValue(std::string && id) override
		{
			if (auto teo = std::dynamic_pointer_cast<T>(sharedObjects.at(id))) {
				this->v = std::move(teo);
			}
			else {
				throw std::runtime_error("Named type doesn't cast to target type");
			}
		}
	};
}

#define STORE_TYPE store.persistType<std::decay_t<decltype(*this)>>()
#define STORE_MEMBER(mbr) store.persistValue(#mbr, mbr)

#endif
