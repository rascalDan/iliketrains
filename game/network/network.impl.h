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

template<typename T>
void
NetworkOf<T>::joinLinks(const LinkPtr & l) const
{
	for (const auto & ol : links.objects) {
		Network::joinLinks(l, ol);
	}
}
