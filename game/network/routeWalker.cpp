#include "routeWalker.h"
#include <array>
#include <game/network/link.h>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

RouteWalker::RouteWalker() : solutionLength {std::numeric_limits<float>::max()} { }

RouteWalker::Solution
RouteWalker::findRouteTo(const Link::End & currentEnd, const Node::Ptr & dest)
{
	findRouteTo(currentEnd, dest, 0);
	return bestSolution;
}

void
// NOLINTNEXTLINE(misc-no-recursion)
RouteWalker::findRouteTo(const Link::End & currentEnd, const Node::Ptr & dest, float length)
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
	for (const auto & next : currentEnd.nexts) {
		const auto link = next.first.lock();
		currentSolution.emplace_back(next);
		findRouteTo(link->ends[!next.second], dest, length + link->length);
		currentSolution.pop_back();
	}
	visited.erase(&currentEnd);
}
