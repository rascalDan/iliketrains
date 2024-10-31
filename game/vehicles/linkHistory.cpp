#include "linkHistory.h"
#include "game/network/link.h"
#include <memory>
#include <optional>

LinkHistory::Entry
LinkHistory::add(const Link::WPtr & l, unsigned char d)
{
	constexpr auto HISTORY_KEEP_LENGTH = 500'000.F;
	while (const auto newLength = [this]() -> std::optional<decltype(totalLen)> {
		if (!links.empty()) {
			const auto newLength = totalLen - links.back().first.lock()->length;
			if (newLength >= HISTORY_KEEP_LENGTH) {
				return newLength;
			}
		}
		return std::nullopt;
	}()) {
		totalLen = newLength.value();
		links.pop_back();
	}
	links.insert(links.begin(), {l, d});
	const auto lp = l.lock();
	totalLen += lp->length;
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
