#include "saxParse-persistence.h"

namespace Persistence {

	void
	SAXParsePersistence::loadStateInternal(FILE * in)
	{
		stk.top()->beforeValue(stk);
		stk.top()->beginObject(stk);
		parseFile(in);
		stk.pop();
		stk.pop();
	}

	void
	SAXParsePersistence::elementOpen(mxml_node_t * n)
	{
		stk.push(stk.top()->select(mxmlGetElement(n)));
		stk.top()->beforeValue(stk);
		stk.top()->beginObject(stk);
		for (int attrCount = mxmlElementGetAttrCount(n), attrIdx {0}; attrIdx < attrCount; ++attrIdx) {
			const char *name, *value = mxmlElementGetAttrByIndex(n, attrIdx, &name);
			auto sel = stk.top()->select(name);
			sel->beforeValue(stk);
			sel->setValue(std::string {value});
		}
	}

	void
	SAXParsePersistence::elementClose(mxml_node_t *)
	{
		stk.top()->endObject(stk);
		stk.top()->endObject(stk);
	}

	void
	SAXParsePersistence::data(mxml_node_t *)
	{
	}

	void
	SAXParsePersistence::directive(mxml_node_t *)
	{
	}

	void
	SAXParsePersistence::cdata(mxml_node_t *)
	{
	}
}
