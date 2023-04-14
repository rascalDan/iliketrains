#include "texturePacker.h"
#include "collections.hpp"
#include <GL/glew.h>
#include <algorithm>
#include <cstdio>
#include <glm/common.hpp>
#include <numeric>
#include <ostream>
#include <set>

TexturePacker::TexturePacker(std::span<const Image> in) :
	inputImages {std::move(in)}, sortedIndexes {vectorOfN(inputImages.size(), size_t {})}
{
	std::sort(sortedIndexes.rbegin(), sortedIndexes.rend(), [this](const auto a, const auto b) {
		return area(inputImages[a]) < area(inputImages[b]);
	});
	int mts;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &mts);
	maxTextureSize = static_cast<unsigned int>(mts);
}

TexturePacker::Result
TexturePacker::pack() const
{
	return pack(minSize());
}

TexturePacker::Result
TexturePacker::pack(Size size) const
{
	if (size.x > maxTextureSize || size.y > maxTextureSize) {
		return {};
	}
	using Spaces = std::set<Space>;
	Spaces spaces {{{}, size}};

	Positions result(inputImages.size());
	for (const auto & idx : sortedIndexes) {
		const auto & image = inputImages[idx];
		if (const auto spaceItr = std::find_if(spaces.begin(), spaces.end(),
					[image](const Space & s) {
						return image.x <= s.size.x && image.y <= s.size.y;
					});
				spaceItr != spaces.end()) {
			auto space = *spaceItr;
			result[idx] = space.position;
			spaces.erase(spaceItr);
			if (space.size.x > image.x) {
				spaces.emplace(Position {space.position.x + image.x, space.position.y},
						Size {space.size.x - image.x, image.y});
			}
			if (space.size.y > image.y) {
				spaces.emplace(Position {space.position.x, space.position.y + image.y},
						Size {space.size.x, space.size.y - image.y});
			}
		}
		else {
			const auto x = pack({size.x * 2, size.y}), y = pack({size.x, size.y * 2});
			if (!x.first.empty() && (y.first.empty() || area(x.second) < area(y.second))) {
				return x;
			}
			else if (!y.first.empty()) {
				return y;
			}
			return {};
		}
	}
	if (GLEW_ARB_texture_non_power_of_two) {
		// Crop the size back to minimum size
		size = std::transform_reduce(
				result.begin(), result.end(), inputImages.begin(), Size {},
				[](auto && max, auto && limit) {
					return glm::max(max, limit);
				},
				[](auto && pos, auto && size) {
					return pos + size;
				});
	}

	return {result, size};
}

TexturePacker::Size
TexturePacker::minSize() const
{
	return std::accumulate(inputImages.begin(), inputImages.end(), Size {1}, [](Size size, const Image & i) {
		while (size.x < i.x) {
			size.x *= 2;
		}
		while (size.y < i.y) {
			size.y *= 2;
		}
		return size;
	});
}

decltype(TexturePacker::Size::x)
TexturePacker::area(const Size & size)
{
	return size.x * size.y;
}
