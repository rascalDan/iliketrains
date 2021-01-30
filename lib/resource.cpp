#include "resource.h"
#include <utility>

std::filesystem::path base {std::filesystem::current_path() / "res"};

void
Resource::setBasePath(std::filesystem::path newbase)
{
	base = std::move(newbase);
}

std::filesystem::path
Resource::mapPath(const std::filesystem::path & rel)
{
	return std::filesystem::canonical(base / rel);
}
