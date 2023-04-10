#include "foliage.h"
#include "gfx/gl/sceneShader.h"
#include "gfx/gl/shadowMapper.h"
#include "gfx/models/texture.h"

bool
Foliage::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_HELPER(bodyMesh, Asset::MeshConstruct) && Asset::persist(store);
}

void
Foliage::postLoad()
{
	texture = getTexture();
}

void
Foliage::render(const SceneShader & shader, const Location & loc) const
{
	shader.basic.use(loc);
	if (texture) {
		texture->bind();
	}
	bodyMesh->Draw();
}

void
Foliage::shadows(const ShadowMapper & mapper, const Location & loc) const
{
	mapper.dynamicPoint.use(loc);
	bodyMesh->Draw();
}
