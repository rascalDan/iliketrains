#include "illuminator.h"
#include "gfx/gl/sceneShader.h"
#include "gfx/models/texture.h" // IWYU pragma: keep
#include "util.h"
#include <location.h>

static_assert(std::is_constructible_v<Illuminator>);

std::weak_ptr<glVertexArray> Illuminator::commonInstanceVAO;

std::any
Illuminator::createAt(const Location & position) const
{
	return std::make_shared<InstanceVertices<InstanceVertex>::InstanceProxy>(
			instances.acquire(locationData->acquire(position)));
}

bool
Illuminator::SpotLight::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_MEMBER(position) && STORE_MEMBER(direction) && STORE_MEMBER(colour) && STORE_MEMBER(kq)
			&& STORE_MEMBER(arc);
}

bool
Illuminator::PointLight::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_MEMBER(position) && STORE_MEMBER(colour) && STORE_MEMBER(kq);
}

bool
Illuminator::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_HELPER(bodyMesh, Asset::MeshConstruct)
			&& STORE_HELPER(pointLight, Persistence::Appender<decltype(pointLight)>)
			&& STORE_HELPER(spotLight, Persistence::Appender<decltype(spotLight)>) && Asset::persist(store);
}

void
Illuminator::postLoad()
{
	if (spotLight.empty() && pointLight.empty()) {
		throw std::logic_error {"Illuminator has no lights"};
	}
	texture = getTexture();
	glDebugScope _ {0};
	if (createIfRequired(instanceVAO, commonInstanceVAO)) {
		bodyMesh->configureVAO(*instanceVAO, 0).addAttribs<InstanceVertex, &InstanceVertex::location>(1);
	}
}

void
Illuminator::render(const SceneShader & shader, const Frustum &) const
{
	if (const auto count = instances.size()) {
		glDebugScope _ {*instanceVAO};
		shader.basicInst.use();
		if (texture) {
			texture->bind(0);
		}
		instanceVAO->useBuffer(1, instances);
		bodyMesh->drawInstanced(*instanceVAO, static_cast<GLsizei>(count));
	}
}
