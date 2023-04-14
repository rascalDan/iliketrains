#pragma once

#include "persistence.h"

class AssImp : public Persistence::Persistable {
public:
	using Ptr = std::shared_ptr<AssImp>;

	void postLoad() override;

	bool persist(Persistence::PersistenceStore & store) override;

	std::string path;
};
