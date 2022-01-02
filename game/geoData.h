#pragma once

#include <filesystem>
#include <glm/glm.hpp>
#include <span>
#include <utility>
#include <vector>

class GeoData {
public:
	struct Node {
		float height {-1.5F};
	};

	using Limits = std::pair<glm::ivec2, glm::ivec2>;

	GeoData() = default;
	explicit GeoData(Limits limit, float scale = 10.F);

	void generateRandom();
	void loadFromImages(const std::filesystem::path &, float scale);

	[[nodiscard]] unsigned int at(glm::ivec2) const;
	[[nodiscard]] unsigned int at(int x, int y) const;

	[[nodiscard]] Limits getLimit() const;
	[[nodiscard]] glm::uvec2 getSize() const;
	[[nodiscard]] float getScale() const;
	[[nodiscard]] std::span<const Node> getNodes() const;

protected:
	Limits limit {}; // Base grid limits first(x,y) -> second(x,y)
	glm::uvec2 size {};
	float scale {1};
	std::vector<Node> nodes;
};
