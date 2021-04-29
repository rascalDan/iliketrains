#ifndef PERSISTANCE_H
#define PERSISTANCE_H

#include <functional>
#include <glm/glm.hpp>
#include <iosfwd>
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

namespace Persistanace {
	struct Selection;
	using SelectionPtr = std::unique_ptr<Selection>;
	using Stack = std::stack<SelectionPtr>;

	struct Selection {
		Selection() = default;
		virtual ~Selection() = default;
		DEFAULT_MOVE_COPY(Selection);

		virtual void setValue(float &);
		virtual void setValue(bool &);
		virtual void setValue(const std::nullptr_t &);
		virtual void setValue(std::string &);
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

		void
		setValue(T & evalue) override
		{
			std::swap(this->v, evalue);
		}
	};

	struct PersistanceStore {
		// virtual bool persistType(const std::type_info &) = 0;
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

		virtual bool persist(PersistanceStore & store) = 0;

		static void addFactory(const std::string_view, std::function<std::unique_ptr<Persistable>()>);
		static std::unique_ptr<Persistable> callFactory(const std::string_view);
	};

	template<typename T> struct SelectionT<std::unique_ptr<T>> : public SelectionV<std::unique_ptr<T>> {
		using Ptr = std::unique_ptr<T>;
		struct SelectionObj : public SelectionV<Ptr> {
			struct MakeObjectByTypeName : public SelectionV<Ptr> {
				using SelectionV<Ptr>::SelectionV;

				void
				setValue(std::string & type) override
				{
					auto no = Persistable::callFactory(type);
					if (dynamic_cast<T *>(no.get())) {
						this->v.reset(static_cast<T *>(no.release()));
					}
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
				else {
					if (!this->v) {
						if constexpr (std::is_abstract_v<T>) {
							throw std::runtime_error("cannot select member of null object");
						}
						else {
							this->v = std::make_unique<T>();
						}
					}
					PersistanceStore ps {mbr};
					if (this->v->persist(ps)) {
						throw std::runtime_error("cannot find member: " + mbr);
					}
					return std::move(ps.sel);
				}
			}

			void
			endObject(Stack & stk) override
			{
				if (!this->v) {
					if constexpr (std::is_abstract_v<T>) {
						throw std::runtime_error("cannot default create abstract object");
					}
					else {
						this->v = std::make_unique<T>();
					}
				}
				stk.pop();
			}
		};

		using SelectionV<Ptr>::SelectionV;

		void
		setValue(const std::nullptr_t &) override
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
}

#define STORE_TYPE store.persistType(typeid(*this))
#define STORE_MEMBER(mbr) store.persistValue(#mbr, mbr)

#endif
