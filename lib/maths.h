#pragma once

#include "config/types.h"
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <utility>

template<typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

struct Arc : public std::pair<Angle, Angle> {
	template<glm::length_t Lc, glm::length_t Le, Arithmetic T, glm::qualifier Q>
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

template<glm::length_t D>
constexpr GlobalPosition<D>
operator+(const GlobalPosition<D> & global, const RelativePosition<D> & relative)
{
	return global + GlobalPosition<D>(glm::round(relative));
}

template<glm::length_t D>
constexpr GlobalPosition<D>
operator+(const GlobalPosition<D> & global, const CalcPosition<D> & relative)
{
	return global + GlobalPosition<D>(relative);
}

template<glm::length_t D>
constexpr GlobalPosition<D>
operator-(const GlobalPosition<D> & global, const RelativePosition<D> & relative)
{
	return global - GlobalPosition<D>(glm::round(relative));
}

template<glm::length_t D>
constexpr GlobalPosition<D>
operator-(const GlobalPosition<D> & global, const CalcPosition<D> & relative)
{
	return global - GlobalPosition<D>(relative);
}

template<glm::length_t D, Arithmetic T, glm::qualifier Q>
using DifferenceVector = glm::vec<D, std::conditional_t<std::is_floating_point_v<T>, T, float>, Q>;

template<glm::length_t D, Arithmetic T, glm::qualifier Q>
constexpr DifferenceVector<D, T, Q>
difference(const glm::vec<D, T, Q> & globalA, const glm::vec<D, T, Q> & globalB)
{
	return globalA - globalB;
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
	template<glm::length_t C, glm::length_t R, Arithmetic T, glm::qualifier Q, std::integral I = glm::length_t>
	constexpr auto &
	operator^(glm::mat<C, R, T, Q> & matrix, const glm::vec<2, I> rowCol)
	{
		return matrix[rowCol.x][rowCol.y];
	}

	// Create a matrix for the angle, given the targets into the matrix
	template<glm::length_t D, std::floating_point T, glm::qualifier Q, std::integral I = glm::length_t>
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

template<Arithmetic T, glm::qualifier Q>
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

template<glm::qualifier Q>
constexpr glm::vec<3, int64_t, Q>
crossProduct(const glm::vec<3, int64_t, Q> & valueA, const glm::vec<3, int64_t, Q> & valueB)
{
	return {
			(valueA.y * valueB.z) - (valueA.z * valueB.y),
			(valueA.z * valueB.x) - (valueA.x * valueB.z),
			(valueA.x * valueB.y) - (valueA.y * valueB.x),
	};
}

template<std::integral T, glm::qualifier Q>
constexpr glm::vec<3, T, Q>
crossProduct(const glm::vec<3, T, Q> & valueA, const glm::vec<3, T, Q> & valueB)
{
	return crossProduct<Q>(valueA, valueB);
}

template<std::floating_point T, glm::qualifier Q>
constexpr glm::vec<3, T, Q>
crossProduct(const glm::vec<3, T, Q> & valueA, const glm::vec<3, T, Q> & valueB)
{
	return glm::cross(valueA, valueB);
}

template<Arithmetic R = float, Arithmetic Ta, Arithmetic Tb>
constexpr auto
ratio(const Ta valueA, const Tb valueB)
{
	using Common = std::common_type_t<Ta, Ta>;
	return static_cast<R>((static_cast<Common>(valueA) / static_cast<Common>(valueB)));
}

template<Arithmetic R = float, Arithmetic T, glm::qualifier Q>
constexpr auto
ratio(const glm::vec<2, T, Q> & value)
{
	return ratio<R>(value.x, value.y);
}

template<glm::length_t L = 3, std::floating_point T, glm::qualifier Q>
constexpr auto
perspective_divide(const glm::vec<4, T, Q> & value)
{
	return value / value.w;
}

template<glm::length_t L1, glm::length_t L2, Arithmetic T, glm::qualifier Q>
constexpr glm::vec<L1 + L2, T, Q>
operator||(const glm::vec<L1, T, Q> valueA, const glm::vec<L2, T, Q> valueB)
{
	return {valueA, valueB};
}

template<glm::length_t L, Arithmetic T, glm::qualifier Q>
constexpr glm::vec<L + 1, T, Q>
operator||(const glm::vec<L, T, Q> valueA, const T valueB)
{
	return {valueA, valueB};
}

template<glm::length_t L, std::floating_point T, glm::qualifier Q>
constexpr glm::vec<L, T, Q>
perspectiveMultiply(const glm::vec<L, T, Q> & base, const glm::mat<L + 1, L + 1, T, Q> & mutation)
{
	const auto mutated = mutation * (base || T(1));
	return mutated / mutated.w;
}

template<glm::length_t L, std::floating_point T, glm::qualifier Q>
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

template<Arithmetic T> using CalcType = std::conditional_t<std::is_floating_point_v<T>, T, int64_t>;

template<Arithmetic T, glm::qualifier Q = glm::defaultp>
[[nodiscard]] constexpr std::optional<glm::vec<2, T, Q>>
linesIntersectAt(const glm::vec<2, T, Q> Aabs, const glm::vec<2, T, Q> Babs, const glm::vec<2, T, Q> Cabs,
		const glm::vec<2, T, Q> Dabs)
{
	using CT = CalcType<T>;
	using CVec = glm::vec<2, CT, Q>;
	// Line AB represented as a1x + b1y = c1
	const CVec Brel = Babs - Aabs;
	const CT a1 = Brel.y;
	const CT b1 = -Brel.x;

	// Line CD represented as a2x + b2y = c2
	const CVec Crel = Cabs - Aabs, Del = Dabs - Aabs;
	const CT a2 = Del.y - Crel.y;
	const CT b2 = Crel.x - Del.x;
	const CT c2 = (a2 * Crel.x) + (b2 * Crel.y);

	const auto determinant = (a1 * b2) - (a2 * b1);

	if (determinant == 0) {
		return std::nullopt;
	}
	return Aabs + CVec {(b1 * c2) / -determinant, (a1 * c2) / determinant};
}

template<Arithmetic T, glm::qualifier Q>
std::pair<glm::vec<2, T, Q>, bool>
find_arc_centre(glm::vec<2, T, Q> start, Rotation2D startDir, glm::vec<2, T, Q> end, Rotation2D endDir)
{
	const auto det = endDir.x * startDir.y - endDir.y * startDir.x;
	if (det != 0) { // near parallel line will yield noisy results
		const glm::vec<2, RelativeDistance, Q> d = end - start;
		const auto u = (d.y * endDir.x - d.x * endDir.y) / det;
		return {start + glm::vec<2, T, Q>(startDir * u), u < 0};
	}
	throw std::runtime_error("no intersection");
}

template<Arithmetic T, glm::qualifier Q>
std::pair<glm::vec<2, T, Q>, bool>
find_arc_centre(glm::vec<2, T, Q> start, Angle entrys, glm::vec<2, T, Q> end, Angle entrye)
{
	if (start == end) {
		return {start, false};
	}
	return find_arc_centre(start, sincos(entrys + half_pi), end, sincos(entrye - half_pi));
}

template<Arithmetic T, glm::qualifier Q>
Angle
find_arcs_radius(glm::vec<2, T, Q> start, Rotation2D ad, glm::vec<2, T, Q> end, Rotation2D bd)
{
	using std::sqrt;

	// Calculates path across both arcs along the normals... pythagorean theorem... for some known radius r
	// (2r)^2 = ((m + (X*r)) - (o + (Z*r)))^2 + ((n + (Y*r)) - (p + (W*r)))^2
	// According to symbolabs.com equation tool, that solves for r to give:
	// r=(-2 m X+2 X o+2 m Z-2 o Z-2 n Y+2 Y p+2 n W-2 p W-sqrt((2 m X-2 X o-2 m Z+2 o Z+2 n Y-2 Y p-2 n W+2 p W)^(2)-4
	// (X^(2)-2 X Z+Z^(2)+Y^(2)-2 Y W+W^(2)-4) (m^(2)-2 m o+o^(2)+n^(2)-2 n p+p^(2))))/(2 (X^(2)-2 X Z+Z^(2)+Y^(2)-2 Y
	// W+W^(2)-4))
	// Locally simplified to work relative, removing one half of the problem and operating on relative positions.

	// These exist cos limitations of online formula rearrangement, and I'm OK with that.
	const RelativePosition2D diff {end - start};
	const auto &o {diff.x}, &p {diff.y};
	const auto &X {ad.x}, &Y {ad.y}, &Z {bd.x}, &W {bd.y};

	return (-2 * X * o + 2 * o * Z - 2 * Y * p + 2 * p * W
				   - sqrt(sq(2 * X * o - 2 * o * Z + 2 * Y * p - 2 * p * W)
						   - (4 * (sq(X) - 2 * X * Z + sq(Z) + sq(Y) - 2 * Y * W + sq(W) - 4) * (sq(o) + sq(p)))))
			/ (2 * (sq(X) - 2 * X * Z + sq(Z) + sq(Y) - 2 * Y * W + sq(W) - 4));
}

template<Arithmetic T, glm::qualifier Q>
std::pair<Angle, Angle>
find_arcs_radius(glm::vec<2, T, Q> start, Angle entrys, glm::vec<2, T, Q> end, Angle entrye)
{
	const auto getrad = [&](auto leftOrRight) {
		return find_arcs_radius(start, sincos(entrys + leftOrRight), end, sincos(entrye + leftOrRight));
	};
	return {getrad(-half_pi), getrad(half_pi)};
}

template<Arithmetic T>
auto
midpoint(const std::pair<T, T> & v)
{
	return std::midpoint(v.first, v.second);
}

template<glm::length_t D, std::integral T, glm::qualifier Q>
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
float operator"" _mph(const long double v);
float operator"" _kph(const long double v);

constexpr float
operator"" _degrees(long double degrees)
{
	return static_cast<float>(degrees) * degreesToRads;
}
