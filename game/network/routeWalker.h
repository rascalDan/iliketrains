#ifndef ROUTEWALKER_H
#define ROUTEWALKER_H

#include "link.h"
#include <set>
#include <vector>

class RouteWalker {
public:
	using Solution = std::vector<LinkWPtr>;

	RouteWalker();

	Solution findRouteTo(const Link::End & currentEnd, const NodePtr & dest);

private:
	void findRouteTo(const Link::End & currentEnd, const NodePtr & dest, float length);

	std::set<const Link::End *> visited;
	Solution bestSolution, currentSolution;
	float solutionLength;
};

#endif
