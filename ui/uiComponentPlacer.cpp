#include "uiComponentPlacer.h"
#include <algorithm>

UIComponentPlacer::UIComponentPlacer(glm::vec2 padding, float spacing, glm::length_t axis) :
	padding {padding}, spacing {spacing}, axis {axis}, current {padding[axis]}
{
}

glm::vec2
UIComponentPlacer::next(glm::vec2 size)
{
	glm::vec2 n {};
	n[axis] = current;
	n[1 - axis] = padding[1 - axis];
	current += spacing + size[axis];
	max = std::max(max, size[1 - axis]);
	return n;
}

glm::vec2
UIComponentPlacer::getLimit() const
{
	glm::vec2 n {};
	n[axis] = current + padding[axis];
	n[1 - axis] = max + padding[1 - axis];
	return n;
}
