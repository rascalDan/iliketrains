#ifndef RESOURCE_H
#define RESOURCE_H

#include <filesystem>

class Resource {
public:
	static void setBasePath(std::filesystem::path);
	static std::filesystem::path mapPath(const std::filesystem::path &);
};

#if defined(RESDIR) && defined(BOOST_TEST_MODULE)
class SetResourcePath {
public:
	SetResourcePath()
	{
		Resource::setBasePath(RESDIR);
	}
};
BOOST_GLOBAL_FIXTURE(SetResourcePath);
#endif

#endif
