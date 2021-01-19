#ifndef WORK_H
#define WORK_H

#include <special_members.hpp>

class Work {
public:
	virtual ~Work() = default;
	NO_COPY(Work);
	NO_MOVE(Work);

	virtual void doWork() = 0;
};

#endif
