#include "geoData.h"
#include "gfx/image.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <glm/gtx/intersect.hpp>
#include <initializer_list>
#include <limits>
#include <maths.h>
#include <random>
#include <ray.hpp>
#include <stb/stb_image.h>
#include <stdexcept>
#include <util.h>

GeoData::GeoData(Limits l, float s) :
	limit {std::move(l)}, size {(limit.second - limit.first) + 1}, scale {s}, nodes {[this]() {
		return (static_cast<std::size_t>(size.x * size.y));
	}()}
{
}

void
GeoData::generateRandom()
{
	// We acknowledge this is terrible :)

	// Add hills
	std::mt19937 gen(std::random_device {}());
	std::uniform_int_distribution<> rxpos(limit.first.x + 2, limit.second.x - 2),
			rypos(limit.first.y + 2, limit.second.y - 2);
	std::uniform_int_distribution<> rsize(10, 30);
	std::uniform_real_distribution<float> rheight(1, 3);
	for (int h = 0; h < 500;) {
		const glm::ivec2 hpos {rxpos(gen), rypos(gen)};
		const glm::ivec2 hsize {rsize(gen), rsize(gen)};
		if (const auto lim1 = hpos - hsize; lim1.x > limit.first.x && lim1.y > limit.first.y) {
			if (const auto lim2 = hpos + hsize; lim2.x < limit.second.x && lim2.y < limit.second.y) {
				const auto height = rheight(gen);
				const glm::ivec2 hsizesqrd {hsize.x * hsize.x, hsize.y * hsize.y};
				for (auto y = lim1.y; y < lim2.y; y += 1) {
					for (auto x = lim1.x; x < lim2.x; x += 1) {
						const auto dist {hpos - glm::ivec2 {x, y}};
						const glm::ivec2 distsqrd {dist.x * dist.x, dist.y * dist.y};
						const auto out {rdiv(sq(x - hpos.x), sq(hsize.x)) + rdiv(sq(y - hpos.y), sq(hsize.y))};
						if (out <= 1.0F) {
							auto & node {nodes[at({x, y})]};
							const auto m {1.F / (7.F * out - 8.F) + 1.F};
							node.height += height * m;
						}
					}
				}
				h += 1;
			}
		}
	}
}

void
GeoData::loadFromImages(const std::filesystem::path & fileName, float scale_)
{
	const Image map {fileName.c_str(), STBI_grey};
	size = {map.width, map.height};
	limit = {{0, 0}, size - glm::uvec2 {1, 1}};
	const auto points {size.x * size.y};
	scale = scale_;
	nodes.resize(points);

	std::transform(map.data.data(), map.data.data() + points, nodes.begin(), [](auto d) {
		return Node {(d * 0.1F) - 1.5F};
	});
}

GeoData::Quad
GeoData::quad(glm::vec2 wcoord) const
{
	constexpr static const std::array<glm::vec2, 4> corners {{{0, 0}, {0, 1}, {1, 0}, {1, 1}}};
	return transform_array(transform_array(corners,
								   [coord = (wcoord / scale)](const auto c) {
									   return glm::vec2 {std::floor(coord.x), std::floor(coord.y)} + c;
								   }),
			[this](const auto c) {
				return (c * scale) || nodes[at(c)].height;
			});
}

glm::vec3
GeoData::positionAt(const glm::vec2 wcoord) const
{
	const auto point {quad(wcoord)};
	const glm::vec2 frac = (wcoord - !point.front()) / scale;
	auto edge = [&point, &frac](auto offset) {
		return point[offset].z + ((point[offset + 2].z - point[offset].z) * frac.x);
	};
	const auto heightFloor = edge(0U), heightCeil = edge(1U),
			   heightMid = heightFloor + ((heightCeil - heightFloor) * frac.y);

	return wcoord || heightMid;
}

GeoData::RayTracer::RayTracer(glm::vec2 p0, glm::vec2 p1) : p {glm::floor(p0)}, d {glm::abs(p1)}
{
	using Limits = std::numeric_limits<typename glm::vec2::value_type>;
	static_assert(Limits::has_infinity);

	auto byAxis = [this, p0, p1](auto axis) {
		if (d[axis] == 0) {
			inc[axis] = 0;
			return Limits::infinity();
		}
		else if (p1[axis] > 0) {
			inc[axis] = 1;
			return (std::floor(p0[axis]) + 1.F - p0[axis]) * d[1 - axis];
		}
		else {
			inc[axis] = -1;
			return (p0[axis] - std::floor(p0[axis])) * d[1 - axis];
		}
	};

	error = byAxis(0);
	error -= byAxis(1);
}

glm::vec2
GeoData::RayTracer::next()
{
	glm::vec2 cur {p};

	static constexpr const glm::vec2 m {1, -1};
	const int axis = (error > 0) ? 1 : 0;
	p[axis] += inc[axis];
	error += d[1 - axis] * m[axis];

	return cur;
}

std::optional<glm::vec3>
GeoData::intersectRay(const Ray & ray) const
{
	if (glm::length(!ray.direction) <= 0) {
		return {};
	}
	RayTracer rt {ray.start / scale, ray.direction};
	while (true) {
		const auto n {rt.next() * scale};
		try {
			const auto point = quad(n);
			for (auto offset : {0U, 1U}) {
				glm::vec2 bary;
				float distance;
				if (glm::intersectRayTriangle(ray.start, ray.direction, point[offset], point[offset + 1],
							point[offset + 2], bary, distance)) {
					return point[offset] + ((point[offset + 1] - point[offset]) * bary[0])
							+ ((point[offset + 2] - point[offset]) * bary[1]);
				}
			}
		}
		catch (std::range_error &) {
			const auto rel = n / !ray.direction;
			if (rel.x > 0 && rel.y > 0) {
				return {};
			}
		}
	}

	return {};
}

unsigned int
GeoData::at(glm::ivec2 coord) const
{
	if (coord.x < limit.first.x || coord.x > limit.second.x || coord.y < limit.first.y || coord.y > limit.second.y) {
		throw std::range_error {"Coordinates outside GeoData limits"};
	}
	const glm::uvec2 offset = coord - limit.first;
	return offset.x + (offset.y * size.x);
}

unsigned int
GeoData::at(int x, int y) const
{
	return at({x, y});
}

GeoData::Limits
GeoData::getLimit() const
{
	return limit;
}

float
GeoData::getScale() const
{
	return scale;
}

glm::uvec2
GeoData::getSize() const
{
	return size;
}

std::span<const GeoData::Node>
GeoData::getNodes() const
{
	return nodes;
}
