#include "routeWalker.h"
#include <array>
#include <game/network/link.h>
#include <limits>
#include <memory>
#include <utility>

RouteWalker::RouteWalker() : solutionLength {std::numeric_limits<float>::max()} { }

RouteWalker::Solution
RouteWalker::findRouteTo(const Link::End & currentEnd, const NodePtr & dest)
{
	findRouteTo(currentEnd, dest, 0);
	return bestSolution;
}

void
// NOLINTNEXTLINE(misc-no-recursion)
RouteWalker::findRouteTo(const Link::End & currentEnd, const NodePtr & dest, float length)
{
	if (currentEnd.node == dest && length < solutionLength) {
		bestSolution = currentSolution;
		solutionLength = length;
		return;
	}
	if (visited.contains(&currentEnd)) { // We've been here before
		return;
	}
	visited.insert(&currentEnd);
	for (const auto & nexts : currentEnd.nexts) {
		const auto link = nexts.first.lock();
		currentSolution.push_back(link);
		findRouteTo(link->ends[!nexts.second], dest, length + link->length);
		currentSolution.pop_back();
	}
	visited.erase(&currentEnd);
}
