#pragma once

#include "gfx/image.h"
#include "gfx/models/texture.h"
#include "persistence.h"
#include "stdTypeDefs.h"
#include "worker.h"

class TextureFragment : public Persistence::Persistable, public StdTypeDefs<TextureFragment> {
public:
	std::string id;
	std::string path;
	TextureOptions::MapMode mapmodeU, mapmodeV;
	Worker::WorkPtrT<std::unique_ptr<Image>> image;

private:
	friend Persistence::SelectionPtrBase<Ptr>;
	bool persist(Persistence::PersistenceStore & store) override;
	void postLoad() override;
};
