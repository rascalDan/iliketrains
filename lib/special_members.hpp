#ifndef SPECIAL_MEMBERS_H
#define SPECIAL_MEMBERS_H

#define NO_COPY(TYPE) \
	TYPE(const TYPE &) = delete; \
	void operator=(const TYPE &) = delete

#define NO_MOVE(TYPE) \
	TYPE(TYPE &&) = delete; \
	void operator=(TYPE &&) = delete

#define DEFAULT_MOVE_COPY(TYPE) \
	TYPE(const TYPE &) = default; \
	TYPE(TYPE &&) = default; \
	TYPE & operator=(const TYPE &) = default; \
	TYPE & operator=(TYPE &&) = default

#endif
