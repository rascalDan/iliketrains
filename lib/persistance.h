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

		virtual void operator()(float &);
		virtual void operator()(bool &);
		virtual void operator()(const std::nullptr_t &);
		virtual void operator()(std::string &);
		virtual void BeginArray(Stack &);
		virtual void BeginObject(Stack &);
		virtual void EndObject(Stack &);
		virtual void beforeValue(Stack &);
		virtual SelectionPtr select(const std::string &);
	};

	template<typename T> struct SelectionT : public Selection {
		explicit SelectionT(T & value) : v {value} { }

		void
		beforeValue(Stack &) override
		{
		}

		void
		operator()(T & evalue) override
		{
			std::swap(v, evalue);
		}

		T & v;
	};

	struct PersistanceStore {
		// virtual bool persistType(const std::type_info &) = 0;
		template<typename T>
		inline bool
		persistValue(const std::string_view key, T & value)
		{
			if (key == name) {
				sel = std::make_unique<SelectionT<T>>(std::ref(value));
				return false;
			}
			return true;
		}
		const std::string & name;
		SelectionPtr sel {};
	};

	template<glm::length_t L, typename T, glm::qualifier Q> struct SelectionT<glm::vec<L, T, Q>> : public Selection {
		explicit SelectionT(glm::vec<L, float, Q> & value) : v {value} { }

		void
		BeginArray(Stack &) override
		{
		}

		void
		beforeValue(Stack & stk) override
		{
			stk.push(std::make_unique<SelectionT<T>>(std::ref(v[idx++])));
		}

		glm::vec<L, T, Q> & v;
		glm::length_t idx {0};
	};

	template<typename T> struct SelectionT<std::vector<T>> : public Selection {
		explicit SelectionT(std::vector<T> & value) : v {value} { }

		void
		BeginArray(Stack & stk) override
		{
			stk.push(std::make_unique<SelectionT<T>>(std::ref(v.emplace_back())));
		}

		void
		beforeValue(Stack & stk) override
		{
			stk.push(std::make_unique<SelectionT<T>>(std::ref(v.emplace_back())));
		}

		std::vector<T> & v;
	};

	struct Persistable {
		Persistable() = default;
		virtual ~Persistable() = default;
		DEFAULT_MOVE_COPY(Persistable);

		virtual bool persist(PersistanceStore & store) = 0;

		static void addFactory(const std::string_view, std::function<std::unique_ptr<Persistable>()>);
		static std::unique_ptr<Persistable> callFactory(const std::string_view);
	};

	template<typename T> struct SelectionT<std::unique_ptr<T>> : public Selection {
		using Ptr = std::unique_ptr<T>;
		struct SelectionObj : public Selection {
			struct MakeObjectByTypeName : public Selection {
				explicit MakeObjectByTypeName(Ptr & o) : o {o} { }

				void
				beforeValue(Stack &) override
				{
				}

				void
				operator()(std::string & type) override
				{
					auto no = Persistable::callFactory(type);
					if (dynamic_cast<T *>(no.get())) {
						o.reset(static_cast<T *>(no.release()));
					}
				}

				Ptr & o;
			};

			explicit SelectionObj(Ptr & o) : v {o} { }

			SelectionPtr
			select(const std::string & mbr) override
			{
				using namespace std::literals;
				if (mbr == "@typeid"sv) {
					if (v) {
						throw std::runtime_error("cannot set object type after creation");
					}
					return std::make_unique<MakeObjectByTypeName>(std::ref(v));
				}
				else {
					if (!v) {
						if constexpr (std::is_abstract_v<T>) {
							throw std::runtime_error("cannot select member of null object");
						}
						else {
							v = std::make_unique<T>();
						}
					}
					PersistanceStore ps {mbr};
					if (v->persist(ps)) {
						throw std::runtime_error("cannot find member: " + mbr);
					}
					return std::move(ps.sel);
				}
			}

			void
			EndObject(Stack & stk) override
			{
				if (!v) {
					if constexpr (std::is_abstract_v<T>) {
						throw std::runtime_error("cannot default create abstract object");
					}
					else {
						v = std::make_unique<T>();
					}
				}
				stk.pop();
			}

			Ptr & v;
		};

		explicit SelectionT(Ptr & o) : v {o} { }

		void
		beforeValue(Stack &) override
		{
		}

		void
		operator()(const std::nullptr_t &) override
		{
			v.reset();
		}

		void
		BeginObject(Stack & stk) override
		{
			stk.push(std::make_unique<SelectionObj>(v));
		}

		void
		EndObject(Stack & stk) override
		{
			stk.pop();
		}

		Ptr & v;
	};
}

#define STORE_TYPE store.persistType(typeid(*this))
#define STORE_MEMBER(mbr) store.persistValue(#mbr, mbr)

#endif
