#pragma once

#include <array>
#include <glm/mat4x4.hpp>

class Frustum {
public:
	Frustum(const glm::mat4 & view, const glm::mat4 & projection);

	[[nodiscard]] auto &
	getFrustumPlanes() const
	{
		return planes;
	}

	[[nodiscard]] auto &
	getViewProjection() const
	{
		return viewProjection;
	}

	void updateView(const glm::mat4 & view);

protected:
	static constexpr size_t FACES = 6;
	void updateCache();

	glm::mat4 view, projection;
	glm::mat4 viewProjection, inverseViewProjection;
	std::array<glm::vec4, FACES> planes;
};
