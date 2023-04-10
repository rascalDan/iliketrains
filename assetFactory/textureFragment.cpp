#include "textureFragment.h"
#include "resource.h"
#include <stb/stb_image.h>

bool
TextureFragment::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_MEMBER(id) && STORE_MEMBER(path);
}

void
TextureFragment::postLoad()
{
	image = std::make_unique<Image>(Resource::mapPath(path), STBI_rgb_alpha);
}
