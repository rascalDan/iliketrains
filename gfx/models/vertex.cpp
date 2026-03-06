#include "vertex.h"
#include "gfx/gl/glVertexArray.h"

template<>
Impl::VertexArrayConfigurator &
Impl::VertexArrayConfigurator::addAttribsFor<Vertex>(const GLuint divisor, const glBuffer & buffer)
{
	return addAttribs<Vertex, &Vertex::pos, &Vertex::texCoord, &Vertex::normal, &Vertex::colour, &Vertex::material>(
			divisor, buffer);
}
