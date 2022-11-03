#include "railVehicle.h"
#include "railVehicleClass.h"
#include "train.h"
#include <algorithm>
#include <array>
#include <glm/glm.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/transform.hpp>
#include <location.hpp>
#include <maths.h>
#include <memory>
#include <ray.hpp>

void
RailVehicle::render(const SceneShader & shader) const
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
	location.rot = {vector_pitch(diff), vector_yaw(diff), 0};
	trailBy += 0.6F + overhang;
}

bool
RailVehicle::intersectRay(const Ray & ray, glm::vec2 * baryPos, float * distance) const
{
	constexpr const auto X = 1.35F;
	const auto Y = this->rvClass->length / 2.F;
	constexpr const auto Z = 3.9F;
	const auto moveBy = glm::translate(location.pos) * rotate_ypr(location.rot);
	const std::array<glm::vec3, 8> cornerVertices {{
			moveBy * glm::vec4 {-X, Y, 0, 1}, //  LFB
			moveBy * glm::vec4 {X, Y, 0, 1}, //   RFB
			moveBy * glm::vec4 {-X, Y, Z, 1}, //  LFT
			moveBy * glm::vec4 {X, Y, Z, 1}, //   RFT
			moveBy * glm::vec4 {-X, -Y, 0, 1}, // LBB
			moveBy * glm::vec4 {X, -Y, 0, 1}, //  RBB
			moveBy * glm::vec4 {-X, -Y, Z, 1}, // LBT
			moveBy * glm::vec4 {X, -Y, Z, 1}, //  RBT
	}};
	static constexpr const std::array<glm::uvec3, 10> triangles {{
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
			triangles.begin(), triangles.end(), [&cornerVertices, &ray, &baryPos, &distance](const glm::uvec3 idx) {
				return glm::intersectRayTriangle(ray.start, ray.direction, cornerVertices[idx[0]],
						cornerVertices[idx[1]], cornerVertices[idx[2]], *baryPos, *distance);
			});
}
