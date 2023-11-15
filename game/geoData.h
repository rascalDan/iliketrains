#pragma once

#include "config/types.h"
#include <array>
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
		float height {-1500.F};
	};

	using Quad = std::array<Position3D, 4>;

	using Limits = std::pair<glm::vec<2, int>, glm::vec<2, int>>;

	explicit GeoData(Limits limit, float scale = 10.F);

	void generateRandom();
	void loadFromImages(const std::filesystem::path &, float scale);

	[[nodiscard]] Position3D positionAt(Position2D) const;
	[[nodiscard]] std::optional<Position3D> intersectRay(const Ray &) const;

	[[nodiscard]] unsigned int at(glm::vec<2, int>) const;
	[[nodiscard]] unsigned int at(int x, int y) const;
	[[nodiscard]] Quad quad(Position2D) const;

	[[nodiscard]] Limits getLimit() const;
	[[nodiscard]] glm::vec<2, unsigned int> getSize() const;
	[[nodiscard]] float getScale() const;
	[[nodiscard]] std::span<const Node> getNodes() const;

	class RayTracer {
	public:
		RayTracer(Position2D p0, Position2D p1);

		Position2D next();

	private:
		RayTracer(Position2D p0, Position2D p1, Position2D d);
		RayTracer(Position2D p0, Position2D d, std::pair<float, float>, std::pair<float, float>);
		static std::pair<float, float> byAxis(Position2D p0, Position2D p1, Position2D d, glm::length_t);

		Position2D p;
		const Position2D d;
		float error;
		Position2D inc;
	};

protected:
	Limits limit {}; // Base grid limits first(x,y) -> second(x,y)
	glm::vec<2, unsigned> size {};
	float scale {1};
	std::vector<Node> nodes;
};
