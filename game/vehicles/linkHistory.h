#pragma once

#include <game/network/link.h>
#include <utility>
#include <vector>

class LinkHistory {
public:
	using WEntry = std::pair<Link::WPtr, unsigned char /*dir*/>;
	using Entry = std::pair<Link::CPtr, unsigned char /*dir*/>;
	Entry add(const Link::WPtr &, unsigned char);
	[[nodiscard]] Entry getCurrent() const;
	[[nodiscard]] Entry getAt(float, float *) const;

private:
	std::vector<WEntry> links;
	float totalLen {0.F};
};
