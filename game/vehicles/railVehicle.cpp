#include "railVehicle.h"
#include "railVehicleClass.h"
#include "train.h"
#include <array>
#include <glm/glm.hpp>
#include <location.hpp>
#include <maths.h>
#include <memory>

void
RailVehicle::render(const Shader & shader) const
{
	rvClass->render(shader, location, bogies);
}

void
RailVehicle::move(const Train * t, float & trailBy)
{
	const auto overhang {(rvClass->length - rvClass->wheelBase) / 2};
	const auto & b1Pos = bogies[0] = t->getBogiePosition(t->linkDist, trailBy += overhang);
	const auto & b2Pos = bogies[1] = t->getBogiePosition(t->linkDist, trailBy += rvClass->wheelBase);
	const auto diff = glm::normalize(b2Pos.pos - b1Pos.pos);
	location.pos = (b1Pos.pos + b2Pos.pos) / 2.F;
	location.rot = {-vector_pitch(diff), vector_yaw(diff), 0};
	trailBy += 0.6F + overhang;
}
