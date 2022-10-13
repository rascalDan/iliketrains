#pragma once

#include <game/activity.h>
#include <game/network/link.h>
#include <game/objective.h>

class Orders;

class GoTo : public Objective {
public:
	GoTo(Orders * os, const Link::End &, float, const Node::Ptr & dest);

	[[nodiscard]] ActivityPtr createActivity() const override;
	[[nodiscard]] Link::Next navigate(Link::Nexts::const_iterator, Link::Nexts::const_iterator) const override;

private:
	Link::Nexts links;
	float startDist;
};
