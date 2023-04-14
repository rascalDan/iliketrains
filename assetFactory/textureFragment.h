#pragma once

#include "gfx/image.h"
#include "persistence.h"
#include "stdTypeDefs.hpp"
#include "worker.h"

class TextureFragment : public Persistence::Persistable, public StdTypeDefs<TextureFragment> {
public:
	std::string id;
	std::string path;
	Worker::WorkPtrT<std::unique_ptr<Image>> image;

private:
	friend Persistence::SelectionPtrBase<Ptr>;
	bool persist(Persistence::PersistenceStore & store) override;
	void postLoad() override;
};
