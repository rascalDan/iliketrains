#pragma once

#include "config/types.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <numeric>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <utility>

template<typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

template<Arithmetic T> using CalcType = std::conditional_t<std::is_floating_point_v<T>, T, int64_t>;
template<Arithmetic T> using DifferenceType = std::conditional_t<std::is_floating_point_v<T>, T, float>;

struct Arc : public std::pair<Angle, Angle> {
	template<glm::length_t Lc, glm::length_t Le, Arithmetic T, glm::qualifier Q = glm::defaultp>
		requires(Lc >= 2, Le >= 2)
	Arc(const glm::vec<Lc, T, Q> & centre, const glm::vec<Le, T, Q> & e0p, const glm::vec<Le, T, Q> & e1p) :
		Arc {RelativePosition2D {e0p.xy() - centre.xy()}, RelativePosition2D {e1p.xy() - centre.xy()}}
	{
	}

	Arc(const RelativePosition2D & dir0, const RelativePosition2D & dir1);
	Arc(Angle anga, Angle angb);

	auto
	operator[](bool getSecond) const
	{
		return getSecond ? second : first;
	}

	[[nodiscard]] constexpr float
	length() const
	{
		return second - first;
	}
};

template<typename T, glm::qualifier Q = glm::defaultp> struct ArcSegment : public Arc {
	using PointType = glm::vec<2, T, Q>;

	constexpr ArcSegment(PointType centre, PointType ep0, PointType ep1);

	PointType centre;
	PointType ep0;
	PointType ep1;
	RelativeDistance radius;

	[[nodiscard]] constexpr std::optional<std::pair<glm::vec<2, T, Q>, Angle>> crossesLineAt(
			const glm::vec<2, T, Q> & lineStart, const glm::vec<2, T, Q> & lineEnd) const;

	[[nodiscard]] constexpr bool
	angleWithinArc(Angle angle) const
	{
		return first <= angle && angle <= second;
	}
};

constexpr const RelativePosition3D up {0, 0, 1}; // NOLINT(readability-identifier-length)
constexpr const RelativePosition3D down {0, 0, -1};
constexpr const RelativePosition3D north {0, 1, 0};
constexpr const RelativePosition3D south {0, -1, 0};
constexpr const RelativePosition3D east {1, 0, 0};
constexpr const RelativePosition3D west {-1, 0, 0};
constexpr auto half_pi {glm::half_pi<float>()};
constexpr auto quarter_pi {half_pi / 2};
constexpr auto pi {glm::pi<float>()}; // NOLINT(readability-identifier-length)
constexpr auto two_pi {glm::two_pi<float>()};
constexpr auto degreesToRads = pi / 180.F;

constexpr auto earthMeanRadius = 6371.01F; // In km
constexpr auto astronomicalUnit = 149597890.F; // In km

// GLM round is not constexpr :( And we can use lround to convert at the same time
template<glm::length_t D, std::floating_point T, glm::qualifier Q>
[[nodiscard]] constexpr glm::vec<D, long, Q>
lround(const glm::vec<D, T, Q> & value)
{
	glm::vec<D, long, Q> out;
	for (glm::length_t axis = 0; axis < D; ++axis) {
		out[axis] = std::lround(value[axis]);
	}
	return out;
}

template<glm::length_t D, std::floating_point T, glm::qualifier Q>
constexpr GlobalPosition<D>
operator+(const GlobalPosition<D> & global, const glm::vec<D, T, Q> & relative)
{
	return global + GlobalPosition<D> {lround(relative)};
}

template<glm::length_t D>
constexpr GlobalPosition<D>
operator+(const GlobalPosition<D> & global, const CalcPosition<D> & relative)
{
	return global + GlobalPosition<D>(relative);
}

template<glm::length_t D, std::floating_point T, glm::qualifier Q>
constexpr GlobalPosition<D>
operator-(const GlobalPosition<D> & global, const glm::vec<D, T, Q> & relative)
{
	return global - GlobalPosition<D> {lround(relative)};
}

template<glm::length_t D>
constexpr GlobalPosition<D>
operator-(const GlobalPosition<D> & global, const CalcPosition<D> & relative)
{
	return global - GlobalPosition<D>(relative);
}

template<glm::length_t D, Arithmetic T, glm::qualifier Q = glm::defaultp>
using DifferenceVector = glm::vec<D, DifferenceType<T>, Q>;

template<glm::length_t D, Arithmetic T, glm::qualifier Q = glm::defaultp>
constexpr DifferenceVector<D, T, Q>
difference(const glm::vec<D, T, Q> & globalA, const glm::vec<D, T, Q> & globalB)
{
	return globalA - globalB;
}

template<glm::length_t D, Arithmetic T, glm::qualifier Q = glm::defaultp>
using CalcVector = glm::vec<D, CalcType<T>, Q>;

template<glm::length_t D, Arithmetic T, glm::qualifier Q = glm::defaultp>
constexpr CalcVector<D, T, Q>
calcDifference(const glm::vec<D, T, Q> & globalA, const glm::vec<D, T, Q> & globalB)
{
	return globalA - globalB;
}

template<glm::length_t D, Arithmetic T, glm::qualifier Q = glm::defaultp>
constexpr auto
distance(const glm::vec<D, T, Q> & pointA, const glm::vec<D, T, Q> & pointB)
{
	return glm::length(difference(pointA, pointB));
}

glm::mat4 flat_orientation(const Rotation3D & diff);

namespace {
	// Helpers
	// C++ wrapper for C's sincosf, but with references, not pointers
	template<std::floating_point T>
	constexpr void
	sincos(T angle, T & sinOut, T & cosOut)
	{
		if consteval {
			sinOut = std::sin(angle);
			cosOut = std::cos(angle);
		}
		else {
			if constexpr (std::is_same_v<T, float>) {
				::sincosf(angle, &sinOut, &cosOut);
			}
			else if constexpr (std::is_same_v<T, double>) {
				::sincos(angle, &sinOut, &cosOut);
			}
			else if constexpr (std::is_same_v<T, long double>) {
				::sincosl(angle, &sinOut, &cosOut);
			}
		}
	}

	template<std::floating_point T, glm::qualifier Q = glm::qualifier::defaultp>
	constexpr auto
	sincos(const T angle)
	{
		glm::vec<2, T, Q> sincosOut {};
		sincos(angle, sincosOut.x, sincosOut.y);
		return sincosOut;
	}

	// Helper to lookup into a matrix given an xy vector coordinate
	template<glm::length_t C, glm::length_t R, Arithmetic T, glm::qualifier Q = glm::defaultp,
			std::integral I = glm::length_t>
	constexpr auto &
	operator^(glm::mat<C, R, T, Q> & matrix, const glm::vec<2, I> rowCol)
	{
		return matrix[rowCol.x][rowCol.y];
	}

	// Create a matrix for the angle, given the targets into the matrix
	template<glm::length_t D, std::floating_point T, glm::qualifier Q = glm::defaultp, std::integral I = glm::length_t>
	constexpr auto
	rotation(const T angle, const glm::vec<2, I> cos1, const glm::vec<2, I> sin1, const glm::vec<2, I> cos2,
			const glm::vec<2, I> negSin1)
	{
		glm::mat<D, D, T, Q> out(1);
		sincos(angle, out ^ sin1, out ^ cos1);
		out ^ cos2 = out ^ cos1;
		out ^ negSin1 = -(out ^ sin1);
		return out;
	}
}

// Create a flat transformation matrix
template<glm::length_t D = 2, glm::qualifier Q = glm::qualifier::defaultp, std::floating_point T>
	requires(D >= 2)
constexpr auto
rotate_flat(const T angle)
{
	return rotation<D, T, Q>(angle, {0, 0}, {0, 1}, {1, 1}, {1, 0});
}

// Create a yaw transformation matrix
template<glm::length_t D = 3, glm::qualifier Q = glm::qualifier::defaultp, std::floating_point T>
	requires(D >= 2)
constexpr auto
rotate_yaw(const T angle)
{
	return rotation<D, T, Q>(angle, {0, 0}, {1, 0}, {1, 1}, {0, 1});
}

// Create a roll transformation matrix
template<glm::length_t D = 3, glm::qualifier Q = glm::qualifier::defaultp, std::floating_point T>
	requires(D >= 3)
constexpr auto
rotate_roll(const T angle)
{
	return rotation<D, T, Q>(angle, {0, 0}, {2, 0}, {2, 2}, {0, 2});
}

// Create a pitch transformation matrix
template<glm::length_t D = 3, glm::qualifier Q = glm::qualifier::defaultp, std::floating_point T>
	requires(D >= 3)
constexpr auto
rotate_pitch(const T angle)
{
	return rotation<D, T, Q>(angle, {1, 1}, {1, 2}, {2, 2}, {2, 1});
}

// Create a combined yaw, pitch, roll transformation matrix
template<glm::length_t D = 3, glm::qualifier Q = glm::qualifier::defaultp, std::floating_point T>
	requires(D >= 3)
constexpr auto
rotate_ypr(const glm::vec<3, T, Q> & angles)
{
	return rotate_yaw<D>(angles.y) * rotate_pitch<D>(angles.x) * rotate_roll<D>(angles.z);
}

template<glm::length_t D = 3, glm::qualifier Q = glm::qualifier::defaultp, std::floating_point T>
	requires(D >= 3)
constexpr auto
rotate_yp(const T yaw, const T pitch)
{
	return rotate_yaw<D>(yaw) * rotate_pitch<D>(pitch);
}

template<glm::length_t D = 3, glm::qualifier Q = glm::qualifier::defaultp, std::floating_point T>
	requires(D >= 3)
constexpr auto
rotate_yp(const glm::vec<2, T, Q> & angles)
{
	return rotate_yp<D>(angles.y, angles.x);
}

template<glm::length_t D, glm::qualifier Q = glm::qualifier::defaultp, std::floating_point T>
	requires(D >= 2)
constexpr auto
vector_yaw(const glm::vec<D, T, Q> & diff)
{
	return std::atan2(diff.x, diff.y);
}

template<glm::length_t D, glm::qualifier Q = glm::qualifier::defaultp, std::floating_point T>
	requires(D >= 3)
constexpr auto
vector_pitch(const glm::vec<D, T, Q> & diff)
{
	return std::atan(diff.z);
}

template<Arithmetic T, glm::qualifier Q = glm::defaultp>
constexpr glm::vec<2, T, Q>
vector_normal(const glm::vec<2, T, Q> & vector)
{
	return {-vector.y, vector.x};
};

template<std::floating_point T>
constexpr auto
round_frac(const T value, const T frac)
{
	return std::round(value / frac) * frac;
}

template<Arithmetic T>
	requires requires(T value) { value * value; }
constexpr auto
sq(T value)
{
	return value * value;
}

template<glm::length_t L, typename T, glm::qualifier Q>
auto
vectorMagSquared(const glm::vec<L, T, Q> & val)
{
	return std::ranges::fold_left(std::views::iota(0, L), T {}, [&val](auto total, auto axis) {
		return total + sq(val[axis]);
	});
}

template<glm::qualifier Q = glm::defaultp>
constexpr glm::vec<3, int64_t, Q>
crossProduct(const glm::vec<3, int64_t, Q> & valueA, const glm::vec<3, int64_t, Q> & valueB)
{
	return {
			(valueA.y * valueB.z) - (valueA.z * valueB.y),
			(valueA.z * valueB.x) - (valueA.x * valueB.z),
			(valueA.x * valueB.y) - (valueA.y * valueB.x),
	};
}

template<std::integral T, glm::qualifier Q = glm::defaultp>
constexpr glm::vec<3, T, Q>
crossProduct(const glm::vec<3, T, Q> & valueA, const glm::vec<3, T, Q> & valueB)
{
	return crossProduct<Q>(valueA, valueB);
}

template<std::floating_point T, glm::qualifier Q = glm::defaultp>
constexpr glm::vec<3, T, Q>
crossProduct(const glm::vec<3, T, Q> & valueA, const glm::vec<3, T, Q> & valueB)
{
	return glm::cross(valueA, valueB);
}

template<Arithmetic R = float, Arithmetic Ta, Arithmetic Tb>
constexpr auto
ratio(const Ta valueA, const Tb valueB)
{
	using Common = std::common_type_t<Ta, Ta, R>;
	return static_cast<R>((static_cast<Common>(valueA) / static_cast<Common>(valueB)));
}

template<Arithmetic R = float, Arithmetic T, glm::qualifier Q = glm::defaultp>
constexpr auto
ratio(const glm::vec<2, T, Q> & value)
{
	return ratio<R>(value.x, value.y);
}

template<glm::length_t L = 3, std::floating_point T, glm::qualifier Q = glm::defaultp>
constexpr auto
perspective_divide(const glm::vec<4, T, Q> & value)
{
	return value / value.w;
}

template<glm::length_t L1, glm::length_t L2, Arithmetic T, glm::qualifier Q = glm::defaultp>
constexpr glm::vec<L1 + L2, T, Q>
operator||(const glm::vec<L1, T, Q> valueA, const glm::vec<L2, T, Q> valueB)
{
	return {valueA, valueB};
}

template<glm::length_t L, Arithmetic T, glm::qualifier Q = glm::defaultp>
constexpr glm::vec<L + 1, T, Q>
operator||(const glm::vec<L, T, Q> valueA, const T valueB)
{
	return {valueA, valueB};
}

template<glm::length_t L, std::floating_point T, glm::qualifier Q = glm::defaultp>
constexpr glm::vec<L, T, Q>
perspectiveMultiply(const glm::vec<L, T, Q> & base, const glm::mat<L + 1, L + 1, T, Q> & mutation)
{
	const auto mutated = mutation * (base || T(1));
	return mutated / mutated.w;
}

template<glm::length_t L, std::floating_point T, glm::qualifier Q = glm::defaultp>
constexpr glm::vec<L, T, Q>
perspectiveApply(glm::vec<L, T, Q> & base, const glm::mat<L + 1, L + 1, T, Q> & mutation)
{
	return base = perspectiveMultiply(base, mutation);
}

template<std::floating_point T>
constexpr T
normalize(T ang)
{
	while (ang > glm::pi<T>()) {
		ang -= glm::two_pi<T>();
	}
	while (ang <= -glm::pi<T>()) {
		ang += glm::two_pi<T>();
	}
	return ang;
}

template<Arithmetic T, std::floating_point D, glm::qualifier Q = glm::defaultp>
[[nodiscard]] constexpr std::optional<glm::vec<2, T, Q>>
linesIntersectAtDirs(const glm::vec<2, T, Q> Aabs, const glm::vec<2, D, Q> Adir, const glm::vec<2, T, Q> Cabs,
		const glm::vec<2, D, Q> Cdir)
{
	const auto abNormal = vector_normal(Adir);
	const auto cdNormal = vector_normal(Cdir);
	const auto determinant = (abNormal.x * cdNormal.y) - (cdNormal.x * abNormal.y);

	if (determinant == 0) {
		return std::nullopt;
	}
	const auto Crel = difference(Cabs, Aabs);
	const auto c2 = (cdNormal.x * Crel.x) + (cdNormal.y * Crel.y);
	return Aabs + vector_normal((abNormal * c2) / determinant);
}

template<Arithmetic T, glm::qualifier Q = glm::defaultp>
[[nodiscard]] constexpr std::optional<glm::vec<2, T, Q>>
linesIntersectAt(const glm::vec<2, T, Q> Aabs, const glm::vec<2, T, Q> Babs, const glm::vec<2, T, Q> Cabs,
		const glm::vec<2, T, Q> Dabs)
{
	return linesIntersectAtDirs(Aabs, difference(Babs, Aabs), Cabs, difference(Dabs, Cabs));
}

template<std::floating_point T> constexpr auto EPSILON = 0.0001F;

template<std::floating_point T>
[[nodiscard]] constexpr auto
isWithinLimit(T lhs, T rhs, T limit = EPSILON<T>)
{
	return std::abs(lhs - rhs) <= limit;
}

template<Arithmetic T, std::floating_point D, glm::qualifier Q = glm::defaultp>
constexpr std::pair<glm::vec<2, T, Q>, D>
find_arc_centre(glm::vec<2, T, Q> start, glm::vec<2, D, Q> entrys, glm::vec<2, T, Q> end)
{
	if (start == end) {
		return {start, 0};
	}
	const auto diffEnds = difference(end, start);
	const auto offset = entrys.x * diffEnds.y - entrys.y * diffEnds.x;
	if (offset == 0.F) {
		return {start, offset};
	}
	const auto midEnds = start + ((end - start) / 2);
	const auto centre = linesIntersectAtDirs(start, vector_normal(entrys), midEnds, vector_normal(diffEnds));
	return {*centre, offset};
}

template<Arithmetic T, glm::qualifier Q = glm::defaultp>
constexpr std::pair<glm::vec<2, T, Q>, float>
find_arc_centre(glm::vec<2, T, Q> start, Angle entrys, glm::vec<2, T, Q> end)
{
	return find_arc_centre(start, sincos(entrys), end);
}

template<Arithmetic T>
auto
midpoint(const std::pair<T, T> & v)
{
	return std::midpoint(v.first, v.second);
}

template<glm::length_t D, std::integral T, glm::qualifier Q = glm::defaultp>
auto
midpoint(const glm::vec<D, T, Q> & valueA, const glm::vec<D, T, Q> & valueB)
{
	return valueA + (valueB - valueA) / 2;
}

// std::pow is not constexpr
template<Arithmetic T>
	requires requires(T n) { n *= n; }
constexpr T
pow(const T base, std::integral auto exp)
{
	T res {1};
	while (exp--) {
		res *= base;
	}
	return res;
}

// Conversions
template<Arithmetic T>
constexpr auto
mph_to_ms(T v)
{
	return v / 2.237L;
}

template<Arithmetic T>
constexpr auto
kph_to_ms(T v)
{
	return v / 3.6L;
}

// ... literals are handy for now, probably go away when we load stuff externally
float operator""_mph(const long double v);
float operator""_kph(const long double v);

constexpr float
operator""_degrees(long double degrees)
{
	return static_cast<float>(degrees) * degreesToRads;
}

// Late implementations due to dependencies
template<typename T, glm::qualifier Q>
constexpr ArcSegment<T, Q>::ArcSegment(PointType centre, PointType ep0, PointType ep1) :
	Arc {centre, ep0, ep1}, centre {centre}, ep0 {ep0}, ep1 {ep1}, radius {::distance(centre, ep0)}
{
}

template<typename T, glm::qualifier Q>
[[nodiscard]] constexpr std::optional<std::pair<glm::vec<2, T, Q>, Angle>>
ArcSegment<T, Q>::crossesLineAt(const glm::vec<2, T, Q> & lineStart, const glm::vec<2, T, Q> & lineEnd) const
{
	// Based on formulas from https://mathworld.wolfram.com/Circle-LineIntersection.html
	const auto lineDiff = difference(lineEnd, lineStart);
	const auto lineLen = glm::length(lineDiff);
	const auto lineRelStart = difference(lineStart, centre);
	const auto lineRelEnd = difference(lineEnd, centre);
	const auto determinant = (lineRelStart.x * lineRelEnd.y) - (lineRelEnd.x * lineRelStart.y);
	const auto discriminant = (radius * radius * lineLen * lineLen) - (determinant * determinant);
	if (discriminant < 0) {
		return std::nullopt;
	}

	const auto rootDiscriminant = std::sqrt(discriminant);
	const auto drdr = lineLen * lineLen;
	const RelativeDistance sgn = (lineDiff.y < 0 ? -1 : 1);
	std::array<std::pair<RelativePosition2D, Angle>, 2> points;
	std::ranges::transform(std::initializer_list {1, -1}, points.begin(), [&](RelativeDistance N) {
		const auto point = RelativePosition2D {((determinant * lineDiff.y) + sgn * lineDiff.x * rootDiscriminant * N),
								   ((-determinant * lineDiff.x) + std::abs(lineDiff.y) * rootDiscriminant * N)}
				/ drdr;
		return std::make_pair(point, vector_yaw(point));
	});
	const auto end
			= std::remove_if(points.begin(), points.end(), [this, lineRelStart, lineDiff, drdr](const auto point) {
				  const auto dot = glm::dot(lineDiff, point.first - lineRelStart);
				  return !angleWithinArc(point.second) || dot < 0 || dot > drdr;
			  });
	if (points.begin() == end) {
		return std::nullopt;
	}
	const auto first = *std::ranges::min_element(points.begin(), end, {}, [lineRelStart](const auto point) {
		return glm::distance(lineRelStart, point.first);
	});
	return std::make_pair(centre + first.first, first.second);
}

namespace {
	template<template<typename> typename Op>
	[[nodiscard]] constexpr auto
	pointLineOp(const GlobalPosition2D point, const GlobalPosition2D end1, const GlobalPosition2D end2)
	{
		return Op {}(CalcDistance(end2.x - end1.x) * CalcDistance(point.y - end1.y),
				CalcDistance(end2.y - end1.y) * CalcDistance(point.x - end1.x));
	}
}

constexpr auto pointLeftOfLine = pointLineOp<std::greater>;
constexpr auto pointLeftOfOrOnLine = pointLineOp<std::greater_equal>;

[[nodiscard]] constexpr bool
linesCross(const GlobalPosition2D lineAend1, const GlobalPosition2D lineAend2, const GlobalPosition2D lineBend1,
		const GlobalPosition2D lineBend2)
{
	return (pointLeftOfLine(lineAend2, lineBend1, lineBend2) == pointLeftOfLine(lineAend1, lineBend2, lineBend1))
			&& (pointLeftOfLine(lineBend1, lineAend1, lineAend2) == pointLeftOfLine(lineBend2, lineAend2, lineAend1));
}

[[nodiscard]] constexpr bool
linesCrossLtR(const GlobalPosition2D lineAend1, const GlobalPosition2D lineAend2, const GlobalPosition2D lineBend1,
		const GlobalPosition2D lineBend2)
{
	return pointLeftOfLine(lineAend2, lineBend1, lineBend2) && pointLeftOfLine(lineAend1, lineBend2, lineBend1)
			&& pointLeftOfLine(lineBend1, lineAend1, lineAend2) && pointLeftOfLine(lineBend2, lineAend2, lineAend1);
}
