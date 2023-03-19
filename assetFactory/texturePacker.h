#pragma once

#include <glm/vec2.hpp>
#include <span>
#include <vector>

class TexturePacker {
public:
	using Position = glm::uvec2;
	using Size = glm::uvec2;

	struct Area {
#ifndef __cpp_aggregate_paren_init
		constexpr Area(Position p, Size s) : position {std::move(p)}, size {std::move(s)} { }
#endif

		Position position;
		Size size;
		bool
		operator<(const Area & other) const
		{
			return area(size) < area(other.size);
		}
	};
	using Image = Size;
	using Space = Area;
	using Positions = std::vector<Position>;
	using Result = std::pair<Positions, Size>;

	TexturePacker(std::span<const Image>);

	Result pack(Size) const;
	Result pack() const;

	Size minSize() const;
	static decltype(Size::x) area(const Size & size);

private:
	std::span<const Image> inputImages;
	std::vector<size_t> sortedIndexes;
};
