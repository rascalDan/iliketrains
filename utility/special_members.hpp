#ifndef SPECIAL_MEMBERS_H
#define SPECIAL_MEMBERS_H

#define NO_COPY(TYPE) \
	TYPE(const TYPE &) = delete; \
	void operator=(const TYPE &) = delete

#define NO_MOVE(TYPE) \
	TYPE(TYPE &&) = delete; \
	void operator=(TYPE &&) = delete

#endif
