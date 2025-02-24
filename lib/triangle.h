#pragma once

#include "config/types.h"
#include "maths.h"
#include <glm/glm.hpp>

template<glm::length_t Dim, Arithmetic T, glm::qualifier Q = glm::defaultp>
struct Triangle : public glm::vec<3, glm::vec<Dim, T, Q>> {
	using Point = glm::vec<Dim, T, Q>;
	using Base = glm::vec<3, glm::vec<Dim, T, Q>>;
	using Base::Base;

	[[nodiscard]] constexpr Point
	operator*(BaryPosition bari) const
	{
		return p(0) + (sideDifference(1) * bari.x) + (sideDifference(2) * bari.y);
	}

	[[nodiscard]] constexpr Point
	centroid() const
	{
		return [this]<glm::length_t... Axis>(std::integer_sequence<glm::length_t, Axis...>) {
			return Point {(p(0)[Axis] + p(1)[Axis] + p(2)[Axis]) / 3 ...};
		}(std::make_integer_sequence<glm::length_t, Dim>());
	}

	[[nodiscard]] constexpr auto
	area() const
		requires(Dim == 3)
	{
		return glm::length(crossProduct(sideDifference(1), sideDifference(2))) / T {2};
	}

	[[nodiscard]] constexpr auto
	area() const
		requires(Dim == 2)
	{
		return std::abs((sideDifference(1).x * sideDifference(2).y) - (sideDifference(2).x * sideDifference(1).y)) / 2;
	}

	[[nodiscard]] constexpr Normal3D
	normal() const
		requires(Dim == 3)
	{
		return crossProduct(sideDifference(1), sideDifference(2));
	}

	[[nodiscard]] constexpr auto
	height()
	{
		return (area() * 2) / ::distance(p(0), p(1));
	}

	[[nodiscard]] constexpr Normal3D
	nnormal() const
		requires(Dim == 3)
	{
		return glm::normalize(normal());
	}

	[[nodiscard]] constexpr auto
	sideDifference(glm::length_t side) const
	{
		return difference(p(side), p(0));
	}

	[[nodiscard]] constexpr auto
	calcSideDifference(glm::length_t side) const
	{
		return calcDifference(p(side), p(0));
	}

	[[nodiscard]] constexpr auto
	angle(glm::length_t corner) const
	{
		return Arc {P(corner), P(corner + 2), P(corner + 1)}.length();
	}

	template<glm::length_t D = Dim>
	[[nodiscard]] constexpr auto
	angleAt(const glm::vec<D, T, Q> pos) const
		requires(D <= Dim)
	{
		for (glm::length_t i {}; i < 3; ++i) {
			if (glm::vec<D, T, Q> {p(i)} == pos) {
				return angle(i);
			}
		}
		return 0.F;
	}

	[[nodiscard]] constexpr auto
	isUp() const
	{
		const auto edgeAB = sideDifference(1);
		const auto edgeAC = sideDifference(2);
		return edgeAB.x * edgeAC.y >= edgeAB.y * edgeAC.x;
	}

	[[nodiscard]] constexpr auto
	p(const glm::length_t idx) const
	{
		return Base::operator[](idx);
	}

	[[nodiscard]] constexpr auto
	P(const glm::length_t idx) const
	{
		return Base::operator[](idx % 3);
	}

	[[nodiscard]] constexpr Point *
	begin()
	{
		return &(Base::x);
	}

	[[nodiscard]] constexpr const Point *
	begin() const
	{
		return &(Base::x);
	}

	[[nodiscard]] constexpr Point *
	end()
	{
		return begin() + 3;
	}

	[[nodiscard]] constexpr const Point *
	end() const
	{
		return begin() + 3;
	}

	[[nodiscard]]
	constexpr auto
	positionOnPlane(const glm::vec<2, T, Q> coord2d) const
		requires(Dim == 3)
	{
		const auto edgeCrossProduct = crossProduct(calcSideDifference(1), calcSideDifference(2));
		return coord2d
				|| static_cast<T>(
						((edgeCrossProduct.x * p(0).x) + (edgeCrossProduct.y * p(0).y) + (edgeCrossProduct.z * p(0).z)
								- (edgeCrossProduct.x * coord2d.x) - (edgeCrossProduct.y * coord2d.y))
						/ edgeCrossProduct.z);
	}

	[[nodiscard]]
	constexpr bool
	containsPoint(const GlobalPosition2D coord) const
	{
		return pointLeftOfOrOnLine(coord, p(0), p(1)) && pointLeftOfOrOnLine(coord, p(1), p(2))
				&& pointLeftOfOrOnLine(coord, p(2), p(0));
	}
};
