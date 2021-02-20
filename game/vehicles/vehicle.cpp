#include "vehicle.h"
#include "game/network/link.h"
#include <memory>
#include <utility>

Vehicle::Vehicle(const LinkPtr & l, float ld) : linkDist {ld}
{
	linkHist.add(l, 0);
}

LinkHistory::Entry
LinkHistory::add(const LinkWPtr & l, unsigned char d)
{
	links.insert(links.begin(), {l, d});
	const auto lp = l.lock();
	totalLen += lp->length;
	while (totalLen >= 1000.F && !links.empty()) {
		totalLen -= links.back().first.lock()->length;
		links.pop_back();
	}
	return {lp, d};
}

LinkHistory::Entry
LinkHistory::getCurrent() const
{
	return {links.front().first.lock(), links.front().second};
}

LinkHistory::Entry
LinkHistory::getAt(float len, float * rem) const
{
	auto litr = links.begin();
	while (len > 0.F && litr != links.end()) {
		litr++;
		if (litr != links.end()) {
			len -= litr->first.lock()->length;
		}
	}
	if (litr == links.end()) {
		litr--;
	}
	if (rem) {
		*rem = -len;
	}
	return {litr->first.lock(), litr->second};
}
