#pragma once

#include "link.h"
#include <set>

class RouteWalker {
public:
	using Solution = Link::Nexts;

	RouteWalker();

	Solution findRouteTo(const Link::End & currentEnd, const NodePtr & dest);

private:
	void findRouteTo(const Link::End & currentEnd, const NodePtr & dest, float length);

	std::set<const Link::End *> visited;
	Solution bestSolution, currentSolution;
	float solutionLength;
};
