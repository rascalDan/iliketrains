#include "vertex.h"
#include "gfx/gl/vertexArrayObject.h"

template<>
VertexArrayObject &
VertexArrayObject::addAttribsFor<Vertex>(const GLuint arrayBuffer, const GLuint divisor)
{
	return addAttribs<Vertex, &Vertex::pos, &Vertex::texCoord, &Vertex::normal, &Vertex::colour, &Vertex::material>(
			arrayBuffer, divisor);
}
