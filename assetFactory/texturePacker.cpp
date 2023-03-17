#include "texturePacker.h"
#include "collections.hpp"
#include <algorithm>
#include <cstdio>
#include <numeric>
#include <ostream>
#include <set>

TexturePacker::TexturePacker(std::span<const Image> in) :
	inputImages {std::move(in)}, sortedIndexes {vectorOfN(inputImages.size(), size_t {})}
{
	std::sort(sortedIndexes.rbegin(), sortedIndexes.rend(), [this](const auto a, const auto b) {
		return area(inputImages[a]) < area(inputImages[b]);
	});
}

TexturePacker::Result
TexturePacker::pack() const
{
	return pack(minSize());
}

TexturePacker::Result
TexturePacker::pack(Size size) const
{
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
			if (size.x < size.y) {
				return pack({size.x * 2, size.y});
			}
			else {
				return pack({size.x, size.y * 2});
			}
		}
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
