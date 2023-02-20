#pragma once

#include <cstdio>

typedef struct _mxml_node_s mxml_node_t;

namespace Persistence {
	class SAXParse {
	public:
		virtual ~SAXParse() = default;

		virtual void elementOpen(mxml_node_t *) = 0;
		virtual void elementClose(mxml_node_t *) = 0;
		virtual void comment(mxml_node_t *);
		virtual void data(mxml_node_t *) = 0;
		virtual void directive(mxml_node_t *) = 0;
		virtual void cdata(mxml_node_t *) = 0;

		void parseFile(FILE * file);
	};
}
