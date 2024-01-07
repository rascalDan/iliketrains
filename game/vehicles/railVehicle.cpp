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

RailVehicle::RailVehicle(RailVehicleClassPtr rvc) :
	RailVehicleClass::Instance {rvc->instances.acquire()}, rvClass {std::move(rvc)},
	location {[this](const BufferedLocation * l) {
		this->get()->body = l->getRotationTransform();
		this->get()->bodyPos = l->position();
	}},
	bogies {{
			{[this](const BufferedLocation * l) {
				 this->get()->front = l->getRotationTransform();
				 this->get()->frontPos = l->position();
			 },
					Position3D {0, rvClass->wheelBase / 2.F, 0}},
			{[this](const BufferedLocation * l) {
				 this->get()->back = l->getRotationTransform();
				 this->get()->backPos = l->position();
			 },
					Position3D {0, -rvClass->wheelBase / 2.F, 0}},
	}}
{
}

void
RailVehicle::move(const Train * t, float & trailBy)
{
	const auto overhang {(rvClass->length - rvClass->wheelBase) / 2};
	const auto & b1Pos = bogies[0] = t->getBogiePosition(t->linkDist, trailBy += overhang);
	const auto & b2Pos = bogies[1] = t->getBogiePosition(t->linkDist, trailBy += rvClass->wheelBase);
	const auto diff = glm::normalize(RelativePosition3D(b2Pos.position() - b1Pos.position()));
	location.setLocation((b1Pos.position() + b2Pos.position()) / 2, {vector_pitch(diff), vector_yaw(diff), 0});
	trailBy += 600.F + overhang;
}

bool
RailVehicle::intersectRay(const Ray<GlobalPosition3D> & ray, BaryPosition & baryPos, RelativeDistance & distance) const
{
	constexpr const auto X = 1350.F;
	const auto Y = this->rvClass->length / 2.F;
	constexpr const auto Z = 3900.F;
	const glm::mat3 moveBy = location.getRotationTransform();
	const auto cornerVertices
			= cuboidCorners(-X, X, -Y, Y, 0.F, Z) * [&moveBy, this](const auto & corner) -> Position3D {
		return location.position() + GlobalPosition3D(moveBy * corner);
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
				return ray.intersectTriangle(
						cornerVertices[idx[0]], cornerVertices[idx[1]], cornerVertices[idx[2]], baryPos, distance);
			});
}
