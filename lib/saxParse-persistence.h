#pragma once

#include "persistence.h"
#include "saxParse.h"
#include <cstdio>
#include <mxml.h>

namespace Persistence {
	class SAXParsePersistence : public SAXParse, ParseBase {
	private:
		template<typename T> struct Root : public Persistable {
			T t {};

			bool
			persist(PersistenceStore & store)
			{
				return STORE_TYPE && STORE_NAME_MEMBER("ilt", t);
			}
		};

		void loadStateInternal(FILE * in);

	public:
		template<typename T>
		auto
		loadState(FILE * in)
		{
			std::unique_ptr<Root<T>> root;
			stk.push(std::make_unique<SelectionT<decltype(root)>>(std::ref(root)));
			loadStateInternal(in);
			return std::move(root->t);
		}

	protected:
		void elementOpen(mxml_node_t * n) override;
		void elementClose(mxml_node_t *) override;
		void data(mxml_node_t *) override;
		void directive(mxml_node_t *) override;
		void cdata(mxml_node_t *) override;
	};
}
