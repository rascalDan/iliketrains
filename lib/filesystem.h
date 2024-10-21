#pragma once

#include "ptr.h"
#include "special_members.h"
#include <cstddef>
#include <cstdio>
#include <sys/types.h>

namespace filesystem {
	class [[nodiscard]] memmap final {
	public:
		memmap(size_t length, int prot, int flags, int fd, off_t offset);
		~memmap();
		NO_MOVE(memmap);
		NO_COPY(memmap);

		template<typename T>
		T *
		get()
		{
			return static_cast<T *>(addr);
		}

		void msync(int flags) const;

	private:
		void * addr;
		size_t length;
	};

	class [[nodiscard]] fh final {
	public:
		fh(const char * path, int flags, mode_t mode);
		~fh();
		NO_MOVE(fh);
		NO_COPY(fh);

		void truncate(size_t size);
		memmap mmap(size_t length, size_t offset, int prot, int flags);

	private:
		int h;
	};

	FILE * checked_fopen(const char * pathname, const char * mode);
	using FileStar = wrapped_ptrt<FILE, &checked_fopen, &fclose>;
}
