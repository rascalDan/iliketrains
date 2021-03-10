#include "network.h"
#include <gfx/gl/shader.h>
#include <gfx/models/texture.h>

template<typename T>
void
NetworkOf<T>::render(const Shader & shader) const
{
	shader.setModel(Location {}, Shader::Program::StaticPos);
	texture->Bind();
	links.apply(&T::render, shader);
}
