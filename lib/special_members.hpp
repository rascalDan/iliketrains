#pragma once

#define NO_COPY(TYPE) \
	TYPE(const TYPE &) = delete; \
	void operator=(const TYPE &) = delete

#define NO_MOVE(TYPE) \
	TYPE(TYPE &&) = delete; \
	void operator=(TYPE &&) = delete

#define DEFAULT_MOVE(TYPE) \
	TYPE(TYPE &&) noexcept = default; \
	TYPE & operator=(TYPE &&) noexcept = default

#define DEFAULT_COPY(TYPE) \
	TYPE(const TYPE &) = default; \
	TYPE & operator=(const TYPE &) = default

#define DEFAULT_MOVE_COPY(TYPE) \
	DEFAULT_MOVE(TYPE); \
	DEFAULT_COPY(TYPE)

#define DEFAULT_MOVE_NO_COPY(TYPE) \
	DEFAULT_MOVE(TYPE); \
	NO_COPY(TYPE)

#define CUSTOM_MOVE(TYPE) \
	TYPE(TYPE &&) noexcept; \
	TYPE & operator=(TYPE &&) noexcept
