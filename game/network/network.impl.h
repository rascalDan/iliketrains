#include "network.h"
#include <gfx/gl/shader.h>
#include <gfx/models/texture.h>

template<typename T>
void
NetworkOf<T>::render(const Shader & shader) const
{
	if constexpr (std::is_base_of_v<Renderable, T>) {
		shader.setModel(Location {}, Shader::Program::StaticPos);
		texture->Bind();
		links.apply(&Renderable::render, shader);
	}
}

template<typename T>
void
NetworkOf<T>::joinLinks(const LinkPtr & l) const
{
	for (const auto & ol : links.objects) {
		Network::joinLinks(l, ol);
	}
}

template<typename T>
LinkPtr
NetworkOf<T>::intersectRayLinks(const Ray & ray) const
{
	// Click link
	if (const auto link = std::find_if(links.objects.begin(), links.objects.end(),
				[&ray](const std::shared_ptr<T> & link) {
					return link->intersectRay(ray);
				});
			link != links.objects.end()) {
		return *link;
	}
	return {};
}

template<typename T>
LinkPtr
NetworkOf<T>::addStraight(glm::vec3 n1, glm::vec3 n2)
{
	return addLink<typename T::StraightLink>(n1, n2);
}
