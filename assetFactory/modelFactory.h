#pragma once

#include "shape.h"

class ModelFactory {
public:
	using Shapes = std::map<std::string, Shape::CPtr, std::less<>>;

	ModelFactory();

	Shapes shapes;
};
