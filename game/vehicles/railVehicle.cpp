#include "railVehicle.h"
#include "railVehicleClass.h"
#include "train.h"
#include <algorithm>
#include <array>
#include <basicShapes.h>
#include <glm/glm.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/transform.hpp>
#include <location.h>
#include <maths.h>
#include <ray.h>

RailVehicle::RailVehicle(RailVehicleClassPtr rvc, GlobalPosition3D position) :
	RailVehicleClass::Instance {rvc->instances.acquire(
			RailVehicleClass::commonLocationData.lock()->acquire(Location {.pos = position, .rot = {}}),
			RailVehicleClass::commonLocationData.lock()->acquire(
					Location {.pos = position + RelativePosition3D {0, rvc->wheelBase / 2.F, 0}, .rot = {}}),
			RailVehicleClass::commonLocationData.lock()->acquire(
					Location {.pos = position + RelativePosition3D {0, -rvc->wheelBase / 2.F, 0}, .rot = {}}))},
	rvClass {std::move(rvc)}
{
}

void
RailVehicle::move(const Train * t, float & trailBy)
{
	const auto overhang {(rvClass->length - rvClass->wheelBase) / 2};
	const auto & b1Pos = *(get()->front = t->getBogiePosition(t->linkDist, trailBy += overhang));
	const auto & b2Pos = *(get()->back = t->getBogiePosition(t->linkDist, trailBy += rvClass->wheelBase));
	const auto diff = glm::normalize(difference(b1Pos.position, b2Pos.position));
	get()->body = Location {
			.pos = midpoint(b1Pos.position, b2Pos.position), .rot = {vector_pitch(diff), vector_yaw(diff), 0}};
	trailBy += 600.F + overhang;
}

Location
RailVehicle::getLocation() const
{
	return {.pos = get()->body->position, .rot = get()->body->rotation};
}

bool
RailVehicle::intersectRay(const Ray<GlobalPosition3D> & ray, BaryPosition & baryPos, RelativeDistance & distance) const
{
	constexpr const auto X = 1350.F;
	const auto Y = this->rvClass->length / 2.F;
	constexpr const auto Z = 3900.F;
	const auto cornerVertices
			= cuboidCorners(-X, X, -Y, Y, 0.F, Z) * [body = this->get()->body.get()](const auto & corner) {
				  return body->position + (body->rotationMatrix * corner);
			  };
	static constexpr const std::array<glm::vec<3, uint8_t>, 10> triangles {{
			// Front
			{0, 1, 2},
			{1, 2, 3},
			// Left
			{0, 2, 4},
			{2, 4, 6},
			// Back
			{4, 5, 6},
			{5, 6, 7},
			// Right
			{1, 3, 5},
			{3, 5, 7},
			// Top
			{2, 3, 6},
			{3, 6, 7},
	}};
	return std::any_of(
			triangles.begin(), triangles.end(), [&cornerVertices, &ray, &baryPos, &distance](const auto & idx) {
				if (const auto inter = ray.intersectTriangle(
							cornerVertices[idx[0]], cornerVertices[idx[1]], cornerVertices[idx[2]])) {
					baryPos = inter->bary;
					distance = inter->distance;
					return true;
				};
				return false;
			});
}
