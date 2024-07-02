#pragma once

#include <special_members.h>

class ApplicationBase {
public:
	ApplicationBase();
	virtual ~ApplicationBase();

	NO_COPY(ApplicationBase);
	NO_MOVE(ApplicationBase);

private:
	void initSDL() const;
	void initImGUI() const;
};
