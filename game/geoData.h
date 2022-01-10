#pragma once

#include <filesystem>
#include <glm/glm.hpp>
#include <optional>
#include <span>
#include <utility>
#include <vector>

class Ray;

class GeoData {
public:
	struct Node {
		float height {-1.5F};
	};
	using Quad = std::array<glm::vec3, 4>;

	using Limits = std::pair<glm::ivec2, glm::ivec2>;

	GeoData() = default;
	explicit GeoData(Limits limit, float scale = 10.F);

	void generateRandom();
	void loadFromImages(const std::filesystem::path &, float scale);

	[[nodiscard]] glm::vec3 positionAt(glm::vec2) const;
	[[nodiscard]] std::optional<glm::vec3> intersectRay(const Ray &) const;

	[[nodiscard]] unsigned int at(glm::ivec2) const;
	[[nodiscard]] unsigned int at(int x, int y) const;
	[[nodiscard]] Quad quad(glm::vec2) const;

	[[nodiscard]] Limits getLimit() const;
	[[nodiscard]] glm::uvec2 getSize() const;
	[[nodiscard]] float getScale() const;
	[[nodiscard]] std::span<const Node> getNodes() const;

	class RayTracer {
	public:
		RayTracer(glm::vec2 p0, glm::vec2 p1);

		glm::vec2 next();

	private:
		glm::vec2 p;
		const glm::vec2 d;
		float error;
		glm::vec2 inc;
	};

protected:
	Limits limit {}; // Base grid limits first(x,y) -> second(x,y)
	glm::uvec2 size {};
	float scale {1};
	std::vector<Node> nodes;
};
