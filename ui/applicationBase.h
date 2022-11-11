#pragma once

#include <special_members.hpp>

class ApplicationBase {
public:
	ApplicationBase();
	virtual ~ApplicationBase();

	NO_COPY(ApplicationBase);
	NO_MOVE(ApplicationBase);
};
