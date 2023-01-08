#include "filesystem.h"
#include <fcntl.h>
#include <filesystem>
#include <string>
#include <sys/mman.h>
#include <system_error>
#include <unistd.h>

namespace filesystem {
	template<typename... Args>
	[[noreturn]] static void
	throw_filesystem_error(std::string operation, int err, Args &&... args)
	{
		throw std::filesystem::filesystem_error {
				std::move(operation), std::forward<Args>(args)..., std::error_code {err, std::system_category()}};
	}

	memmap::memmap(size_t length, int prot, int flags, int fd, off_t offset) :
		addr {mmap(nullptr, length, prot, flags, fd, offset)}, length {length}
	{
		if (addr == MAP_FAILED) {
			throw std::filesystem::filesystem_error {"mmap", std::error_code {errno, std::system_category()}};
		}
	}

	memmap::~memmap()
	{
		::munmap(addr, length);
	}

	void
	memmap::msync(int flags) const
	{
		if (::msync(addr, length, flags)) {
			throw_filesystem_error("msync", errno);
		}
	}

	// NOLINTNEXTLINE(hicpp-vararg)
	fh::fh(const char * path, int flags, int mode) : h {open(path, flags, mode)}
	{
		if (h == -1) {
			throw_filesystem_error("open", errno, path);
		}
	}

	fh::~fh()
	{
		::close(h);
	}

	void
	fh::truncate(size_t size)
	{
		if (::ftruncate(h, static_cast<off_t>(size))) {
			throw_filesystem_error("ftruncate", errno);
		}
	}

	memmap
	fh::mmap(size_t length, size_t offset, int prot, int flags)
	{
		return memmap {length, prot, flags, h, static_cast<off_t>(offset)};
	}
}
