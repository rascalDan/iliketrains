#pragma once

#include "persistence.h"
#include "stdTypeDefs.hpp"

class TextureFragment : public Persistence::Persistable, public StdTypeDefs<TextureFragment> {
public:
	std::string id;
	std::string path;

private:
	friend Persistence::SelectionPtrBase<Ptr>;
	bool persist(Persistence::PersistenceStore & store) override;
};
