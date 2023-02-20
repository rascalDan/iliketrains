#include "saxParse.h"
#include "mxml.h"

namespace Persistence {
	void
	SAXParse::comment(mxml_node_t *)
	{
		// Default to just ignore comments
	}

	void
	SAXParse::parseFile(FILE * file)
	{
		mxmlSAXLoadFile(
				nullptr, file, MXML_TEXT_CALLBACK,
				[](mxml_node_t * n, mxml_sax_event_t e, void * data) {
					SAXParse * self = static_cast<SAXParse *>(data);
					switch (e) {
						case MXML_SAX_ELEMENT_OPEN:
							return self->elementOpen(n);
							break;
						case MXML_SAX_ELEMENT_CLOSE:
							return self->elementClose(n);
							break;
						case MXML_SAX_COMMENT:
							return self->comment(n);
							break;
						case MXML_SAX_DATA:
							return self->data(n);
							break;
						case MXML_SAX_DIRECTIVE:
							return self->directive(n);
							break;
						case MXML_SAX_CDATA:;
							return self->cdata(n);
							break;
					}
				},
				this);
	}
}
