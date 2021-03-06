#ifndef LINKHISTORY_H
#define LINKHISTORY_H

#include <game/network/link.h>
#include <utility>
#include <vector>

class LinkHistory {
public:
	using WEntry = std::pair<LinkWPtr, unsigned char /*dir*/>;
	using Entry = std::pair<LinkCPtr, unsigned char /*dir*/>;
	Entry add(const LinkWPtr &, unsigned char);
	[[nodiscard]] Entry getCurrent() const;
	[[nodiscard]] Entry getAt(float, float *) const;

private:
	std::vector<WEntry> links;
	float totalLen {0.F};
};

#endif
