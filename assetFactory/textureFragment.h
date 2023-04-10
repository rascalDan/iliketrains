#pragma once

#include "gfx/image.h"
#include "persistence.h"
#include "stdTypeDefs.hpp"

class TextureFragment : public Persistence::Persistable, public StdTypeDefs<TextureFragment> {
public:
	std::string id;
	std::string path;
	std::unique_ptr<Image> image;

private:
	friend Persistence::SelectionPtrBase<Ptr>;
	bool persist(Persistence::PersistenceStore & store) override;
	void postLoad() override;
};
