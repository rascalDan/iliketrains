#define BOOST_TEST_MODULE test_maths

#include "testHelpers.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <glm/gtx/transform.hpp>
#include <stream_support.h>
#include <string_view>
#include <type_traits>

#include <game/network/link.h>
#include <gfx/camera.h>
#include <glm/glm.hpp>
#include <maths.h>
#include <triangle.h>
#include <tuple>

using vecter_and_angle = std::tuple<glm::vec3, float>;
using angle_pair = std::tuple<float, float>;
//
// STANDARD DEFINITIONS
//
// (x, y) in the 2D plane of geographic coordinates.
// (x, y, z) in the 3D plane, where (x, y) are geographic and z is veritcal.
//
// (x, y, 0) is sea level
// (x, y, +ve) is "up"
static_assert(up.z > 0);
static_assert(down == -up);
// (x, +ve, z) is "north"
static_assert(north.y > 0);
static_assert(south == -north);
// (x, -ve, z) is "south"
static_assert(south.y < 0);
// (+ve, y, z) is "east"
static_assert(east.x > 0);
static_assert(west == -east);
// (-ve, y, z) is "west"
static_assert(west.x < 0);

//
// Therefore, the geographic world exists west -ve to east +ve and from south -ve to north +ve.  Forward shall be
// considered +ve motion; the "front" of a vehicle shall have a +ve value in y axis.
//
// An unrotated vehicle shall be facing north, thus forward motion of the vehicle shall increase it's position in the y
// axis.
//
// Positive rotation on the XY plane (y member, yaw, around the down axis, as would be expected for vehicle or building
// on flat land) shall be clockwise, in radians. Cycles shall be considered equal; 0 == 2pi, pi == -pi, 1/4pi == -3/4pi.

BOOST_DATA_TEST_CASE(test_vector_yaw,
		boost::unit_test::data::make<vecter_and_angle>(
				{{up, 0}, {north, 0}, {south, pi}, {west, -half_pi}, {east, half_pi}, {north + east, quarter_pi},
						{south + east, quarter_pi * 3}, {north + west, -quarter_pi}, {south + west, -quarter_pi * 3}}),
		v, a)
{
	BOOST_CHECK_CLOSE(vector_yaw(v), a, 1.F);
}

BOOST_DATA_TEST_CASE(test_angle_normalize,
		boost::unit_test::data::make<angle_pair>({
				{0, 0},
				{two_pi, 0},
				{-two_pi, 0},
				{half_pi, half_pi},
				{-half_pi, -half_pi},
				{half_pi * 3, -half_pi},
				{-half_pi * 3, half_pi},
		}),
		in, exp)
{
	BOOST_CHECK_CLOSE(normalize(in), exp, 1);
}

// Positive rotation on the YZ plane (x member, pitch, around the east axis relative to its yaw, as would be expected
// for a vehicle travelling forward uphill), in radians. Cycles can be considered non-sense as even in the worst/best
// cases pitch beyond +/- 1/2pi would be crashing.

BOOST_DATA_TEST_CASE(test_vector_pitch,
		boost::unit_test::data::make<vecter_and_angle>({
				{north, 0},
				{east, 0},
				{south, 0},
				{west, 0},
				{north + up, quarter_pi},
				{east + up, quarter_pi},
				{south + up, quarter_pi},
				{west + up, quarter_pi},
				{north - up, -quarter_pi},
				{east - up, -quarter_pi},
				{south - up, -quarter_pi},
				{west - up, -quarter_pi},
				{north + west - up, -quarter_pi},
				{north + west + up, quarter_pi},
		}),
		v, a)
{
	BOOST_CHECK_CLOSE(vector_pitch(v), a, 1.F);
}

// Positive rotation on the ZX plane (z member, roll, around Y axis relative to its yaw and pitch, as would be expected
// for an aircraft banking/turning right), in radians. Cycles can be considered non-sense as even in the worst/best
// cases pitch beyond +/- 1/2pi would be crashing.

// The ILT functions rotate_yaw, rotate_pitch and rotate_roll provide a simple equivelent to glm::rotate around the
// stated axis.
const auto angs = boost::unit_test::data::make({pi, half_pi, two_pi, quarter_pi, -pi, -half_pi, -quarter_pi, 0.F})
		* boost::unit_test::data::make(0);
const auto random_angs = boost::unit_test::data::random(-two_pi, two_pi) ^ boost::unit_test::data::xrange(1000);
const auto rots = boost::unit_test::data::make<std::tuple<glm::vec3, glm::mat4 (*)(float), std::string_view>>({
		{down, rotate_yaw<4>, "yaw"},
		{east, rotate_pitch<4>, "pitch"},
		{north, rotate_roll<4>, "roll"},
});

BOOST_DATA_TEST_CASE(test_rotations, (angs + random_angs) * rots, angle, ai, axis, ilt_func, name)
{
	(void)ai;

	BOOST_TEST_CONTEXT(name) {
		const auto g {glm::rotate(angle, axis)}, ilt {ilt_func(angle)};
		for (glm::length_t c = 0; c < 4; c++) {
			BOOST_TEST_CONTEXT(c) {
				for (glm::length_t r = 0; r < 4; r++) {
					BOOST_TEST_CONTEXT(r) {
						BOOST_CHECK_CLOSE(g[c][r], ilt[c][r], 0.0001);
					}
				}
			}
		}
	}
}

// An arc shall be defined as a centre point, start point and end point. The arc shall progress positively from start to
// end in a clockwise manner. Arc start shall be the yaw from centre to start, arc end shall be greater than arc start.
using pos3_to_arc = std::tuple<glm::vec3, glm::vec3, glm::vec3, Arc>;

BOOST_DATA_TEST_CASE(test_create_arc,
		boost::unit_test::data::make<pos3_to_arc>({
				{{0, 0, 0}, north, east, {0, half_pi}},
				{{0, 0, 0}, west, east, {-half_pi, half_pi}},
				{{0, 0, 0}, south, east, {pi, half_pi * 5}},
				{{0, 0, 0}, east, north, {half_pi, two_pi}},
				{{0, 0, 0}, south, north, {pi, two_pi}},
				{{0, 0, 0}, east, south, {half_pi, pi}},
		}),
		c, s, e, a)
{
	const Arc arc {c, s, e};
	BOOST_REQUIRE_LT(arc.first, arc.second);
	BOOST_CHECK_CLOSE(arc.first, a.first, 1.F);
	BOOST_CHECK_CLOSE(arc.second, a.second, 1.F);
}

using fac = std::tuple<glm::vec2, float, glm::vec2, float, glm::vec2, bool>;

BOOST_DATA_TEST_CASE(test_find_arc_centre,
		boost::unit_test::data::make<fac>({
				{{2, 2}, pi, {3, 3}, half_pi, {3, 2}, true},
				{{2, 2}, pi, {1, 3}, -half_pi, {1, 2}, false},
				{{-1100, -1000}, pi, {-900, -800}, half_pi, {-900, -1000}, true},
				{{1100, 1000}, 0, {1050, 900}, pi + 0.92F, {973, 1000}, true},
				{{1050, 900}, 0.92F, {1000, 800}, pi, {1127, 800}, false},
		}),
		s, es, e, ee, exp, lr)
{
	const auto c = find_arc_centre(s, es, e, ee);
	BOOST_CHECK_CLOSE(exp.x, c.first.x, 1);
	BOOST_CHECK_CLOSE(exp.y, c.first.y, 1);
	BOOST_CHECK_EQUAL(lr, c.second);
}

BOOST_AUTO_TEST_CASE(test_find_arcs_radius)
{
	BOOST_CHECK_CLOSE(
			find_arcs_radius(RelativePosition2D {10.32, 26.71}, {0.4, .92}, {2.92, 22.41}, {-0.89, -0.45}), 2.29, 1);
}

struct TestLinkStraight : public LinkStraight {
	explicit TestLinkStraight(glm::vec3 v) :
		Link {{std::make_shared<Node>(GlobalPosition3D {}), vector_yaw(v)}, {std::make_shared<Node>(v), vector_yaw(-v)},
				glm::length(v)}
	{
	}
};

using StraightsData = std::tuple<glm::vec3, float /*angFor*/, float /* angBack*/>;

BOOST_DATA_TEST_CASE(straight1,
		boost::unit_test::data::make<StraightsData>({
				{north, 0, pi},
				{south, pi, 0},
				{east, half_pi, -half_pi},
				{west, -half_pi, half_pi},
		}),
		v, angFor, angBack)
{
	const TestLinkStraight l(v);
	{
		const auto p = l.positionAt(0, 0);
		BOOST_CHECK_EQUAL(p.pos, GlobalPosition3D {});
		BOOST_CHECK_EQUAL(p.rot, glm::vec3(0, angFor, 0));
	}
	{
		const auto p = l.positionAt(0, 1);
		BOOST_CHECK_EQUAL(p.pos, GlobalPosition3D {v});
		BOOST_CHECK_EQUAL(p.rot, glm::vec3(0, angBack, 0));
	}
}

struct TestLinkCurve : public LinkCurve {
	explicit TestLinkCurve(glm::vec3 e0, glm::vec3 e1, glm::vec3 ctr) :
		Link {{std::make_shared<Node>(e0), normalize(vector_yaw(ctr - e0) - half_pi)},
				{std::make_shared<Node>(e1), normalize(vector_yaw(ctr - e1) - half_pi)}, glm::length(e1 - e0)},
		LinkCurve(ctr, glm::length(e0 - ctr), {ctr, e0, e1})
	{
	}
};

using CurvesData = std::tuple<GlobalPosition3D /*e1*/, GlobalPosition3D /*ctr*/, Angle /*angFor*/, Angle /* angBack*/>;

BOOST_DATA_TEST_CASE(curve1,
		boost::unit_test::data::make<CurvesData>({
				{north + east, east, 0, -half_pi},
				{east * 2.F, east, 0, 0},
				{south + east, east, 0, half_pi},
				{south + west, west, pi, half_pi},
		}),
		e1, ctr, angFor, angBack)
{
	{ // One-way...
		const TestLinkCurve l({}, e1, ctr);
		BOOST_CHECK_EQUAL(l.radius, 1.F);
		{
			const auto p = l.positionAt(0, 0);
			BOOST_CHECK_CLOSE_VECI(p.pos, GlobalPosition3D {});
			BOOST_CHECK_CLOSE_VEC(p.rot, glm::vec3(0, angFor, 0));
		}
		{
			const auto p = l.positionAt(0, 1);
			BOOST_CHECK_CLOSE_VECI(p.pos, e1);
			BOOST_CHECK_CLOSE_VEC(p.rot, glm::vec3(0, angBack, 0));
		}
	}

	{ // The other way...
		const TestLinkCurve l(e1, {}, ctr);
		BOOST_CHECK_EQUAL(l.radius, 1.F);
		{
			const auto p = l.positionAt(0, 0);
			const auto angForReversed = normalize(vector_yaw(difference({}, e1)) * 2 - angFor);
			BOOST_CHECK_CLOSE_VECI(p.pos, e1);
			BOOST_CHECK_CLOSE_VEC(p.rot, glm::vec3(0, angForReversed, 0));
		}
		{
			const auto p = l.positionAt(0, 1);
			const auto angBackReversed = normalize(vector_yaw(difference(e1, {})) * 2 - angBack);
			BOOST_CHECK_CLOSE_VECI(p.pos, GlobalPosition3D {});
			BOOST_CHECK_CLOSE_VEC(p.rot, glm::vec3(0, angBackReversed, 0));
		}
	}
}

BOOST_AUTO_TEST_CASE(camera_clicks)
{
	Camera camera {{}, ::half_pi, 1.25F, 1000, 10000000};
	constexpr float centre {0.5F}, right {0.9F}, left {0.1F}, top {1.F}, bottom {0.F};
	camera.setForward(::north);
	BOOST_CHECK_EQUAL(camera.unProject({centre, centre}).start, GlobalPosition3D {});
	BOOST_CHECK_CLOSE_VEC(camera.unProject({centre, centre}).direction, ::north);
	BOOST_CHECK_CLOSE_VEC(camera.unProject({left, centre}).direction, glm::normalize(::north + ::west));
	BOOST_CHECK_CLOSE_VEC(camera.unProject({right, centre}).direction, glm::normalize(::north + ::east));
	BOOST_CHECK_CLOSE_VEC(camera.unProject({centre, top}).direction, glm::normalize(::north + ::up));
	BOOST_CHECK_CLOSE_VEC(camera.unProject({centre, bottom}).direction, glm::normalize(::north + ::down));
	BOOST_CHECK_CLOSE_VEC(camera.unProject({left, top}).direction, glm::normalize(::north + ::west + ::up));
	BOOST_CHECK_CLOSE_VEC(camera.unProject({right, top}).direction, glm::normalize(::north + ::east + ::up));
	BOOST_CHECK_CLOSE_VEC(camera.unProject({left, bottom}).direction, glm::normalize(::north + ::west + ::down));
	BOOST_CHECK_CLOSE_VEC(camera.unProject({right, bottom}).direction, glm::normalize(::north + ::east + ::down));

	camera.setForward(::east);
	BOOST_CHECK_CLOSE_VEC(camera.unProject({centre, centre}).direction, ::east);
	BOOST_CHECK_CLOSE_VEC(camera.unProject({left, centre}).direction, glm::normalize(::north + ::east));
	BOOST_CHECK_CLOSE_VEC(camera.unProject({right, centre}).direction, glm::normalize(::south + ::east));

	camera.setForward(glm::normalize(::north + ::down));
	BOOST_CHECK_CLOSE_VEC(camera.unProject({centre, centre}).direction, glm::normalize(::north + ::down));
	BOOST_CHECK_CLOSE_VEC(camera.unProject({centre, top}).direction, glm::normalize(::north));

	camera.setForward(glm::normalize(::north + ::west + ::down));
	BOOST_CHECK_CLOSE_VEC(camera.unProject({centre, centre}).direction, glm::normalize(::north + ::west + ::down));
	BOOST_CHECK_CLOSE_VEC(camera.unProject({centre, top}).direction, glm::normalize(::north + ::west + ::up * 0.2F));

	camera.setForward(glm::normalize(::north + ::west));
	BOOST_CHECK_CLOSE_VEC(camera.unProject({centre, centre}).direction, glm::normalize(::north + ::west));
	BOOST_CHECK_CLOSE_VEC(camera.unProject({centre, top}).direction, glm::normalize(::north + ::west + ::up * 1.2F));
	BOOST_CHECK_CLOSE_VEC(camera.unProject({right, centre}).direction, glm::normalize(::north));
	BOOST_CHECK_CLOSE_VEC(camera.unProject({left, centre}).direction, glm::normalize(::west));
}

template<typename T = float>
auto
n_test_points_between(std::size_t n = 2, T min = -100.F, T max = 100.F)
{
	return boost::unit_test::data::xrange(n) ^ boost::unit_test::data::random(min, max);
}

BOOST_DATA_TEST_CASE(rayLineDistance,
		n_test_points_between() * // n1x
				n_test_points_between() * // n1y
				n_test_points_between() * // n1z
				n_test_points_between() * // n2x
				n_test_points_between() * // n2y
				n_test_points_between() * // n2z
				n_test_points_between() * // cx
				n_test_points_between() * // cy
				n_test_points_between(), // cz
		i1, n1x, i2, n1y, i3, n1z, i4, n2x, i5, n2y, i6, n2z, i7, cx, i8, cy, i9, cz)
{
	(void)i1;
	(void)i2;
	(void)i3;
	(void)i4;
	(void)i5;
	(void)i6;
	(void)i7;
	(void)i8;
	(void)i9;
	const glm::vec3 n1 {n1x, n1y, n1z}, n2 {n2x, n2y, n2z}, c {cx, cy, cz};

	const auto nstep = n2 - n1;
	for (float along = 0.2F; along <= 0.8F; along += 0.1F) {
		const auto target = n1 + (along * nstep);
		const auto direction = glm::normalize(target - c);
		BOOST_CHECK_LE(Ray<RelativePosition3D>(c, direction).distanceToLine(n1, n2), 0.01F);
	}
}

static_assert(linesIntersectAt(glm::ivec2 {10, 10}, {40, 40}, {10, 80}, {20, 40}).value().x == 24);
static_assert(linesIntersectAt(glm::vec2 {10, 10}, {40, 40}, {10, 80}, {20, 40}).value().y == 24);
static_assert(linesIntersectAt(GlobalPosition2D {311000100, 491100100}, {311050000, 491150000}, {312000100, 491200100},
					  {311000100, 491100100})
					  .value()
		== GlobalPosition2D {311000100, 491100100});
static_assert(!linesIntersectAt(glm::dvec2 {0, 1}, {0, 4}, {1, 8}, {1, 4}).has_value());

BOOST_AUTO_TEST_CASE(triangle2d_helpers)
{
	constexpr static Triangle<2, float> t {{0, 0}, {5, 0}, {5, 5}};

	BOOST_CHECK_CLOSE(t.angle(0), quarter_pi, 0.01F);
	BOOST_CHECK_CLOSE(t.angleAt({0, 0}), quarter_pi, 0.01F);
	BOOST_CHECK_CLOSE(t.angle(1), half_pi, 0.01F);
	BOOST_CHECK_CLOSE(t.angleAt({5, 0}), half_pi, 0.01F);
	BOOST_CHECK_CLOSE(t.angle(2), quarter_pi, 0.01F);
	BOOST_CHECK_CLOSE(t.angleAt({5, 5}), quarter_pi, 0.01F);

	BOOST_CHECK_CLOSE(t.angleAt({0, 1}), 0.F, 0.01F);

	BOOST_CHECK_CLOSE(t.area(), 12.5F, 0.01F);
}

BOOST_AUTO_TEST_CASE(triangle3d_helpers)
{
	constexpr static Triangle<3, float> t {{0, 0, 0}, {5, 0, 0}, {5, 5, 0}};

	BOOST_CHECK_EQUAL(t.nnormal(), up);
	BOOST_CHECK_CLOSE(t.angle(0), quarter_pi, 0.01F);
	BOOST_CHECK_CLOSE(t.angleAt({0, 0, 0}), quarter_pi, 0.01F);
	BOOST_CHECK_CLOSE(t.angle(1), half_pi, 0.01F);
	BOOST_CHECK_CLOSE(t.angleAt({5, 0, 0}), half_pi, 0.01F);
	BOOST_CHECK_CLOSE(t.angle(2), quarter_pi, 0.01F);
	BOOST_CHECK_CLOSE(t.angleAt({5, 5, 0}), quarter_pi, 0.01F);

	BOOST_CHECK_CLOSE(t.angleAt({0, 1, 0}), 0.F, 0.01F);

	BOOST_CHECK_CLOSE(t.area(), 12.5F, 0.01F);
}

using ArcLineIntersectExp = std::pair<GlobalPosition2D, Angle>;
using ArcLineIntersectData = std::tuple<GlobalPosition2D, GlobalPosition2D, GlobalPosition2D, GlobalPosition2D,
		GlobalPosition2D, std::optional<ArcLineIntersectExp>>;

BOOST_DATA_TEST_CASE(arcline_intersection,
		boost::unit_test::data::make<ArcLineIntersectData>({
				{{0, 0}, {0, 100}, {100, 0}, {200, 0}, {0, 200}, std::nullopt},
				{{0, 0}, {0, 100}, {100, 0}, {0, 0}, {10, 10}, std::nullopt},
				{{0, 0}, {0, 100}, {100, 0}, {0, 0}, {100, 100}, ArcLineIntersectExp {{71, 71}, quarter_pi}},
				{{15, 27}, {15, 127}, {115, 27}, {15, 27}, {115, 127}, ArcLineIntersectExp {{86, 98}, quarter_pi}},
				{{0, 0}, {0, 100}, {100, 0}, {0, 0}, {-100, -100}, std::nullopt},
				{{0, 0}, {0, 100}, {100, 0}, {-10, 125}, {125, -10}, ArcLineIntersectExp {{16, 99}, 0.164F}},
				{{0, 0}, {0, 100}, {100, 0}, {125, -10}, {-10, 125}, ArcLineIntersectExp {{99, 16}, 1.407F}},
				{{0, 0}, {0, 100}, {100, 0}, {10, 125}, {125, -10}, ArcLineIntersectExp {{38, 93}, 0.385F}},
				{{0, 0}, {0, 100}, {100, 0}, {12, 80}, {125, -10}, ArcLineIntersectExp {{99, 10}, 1.467F}},
				{{0, 0}, {0, 100}, {100, 0}, {40, 80}, {125, -10}, ArcLineIntersectExp {{98, 18}, 1.387F}},
				{{0, 0}, {0, 100}, {100, 0}, {40, 80}, {80, 20}, std::nullopt},
				{{0, 0}, {0, 100}, {100, 0}, {40, 80}, {80, 80}, ArcLineIntersectExp {{60, 80}, 0.6435F}},
				{{0, 0}, {0, 100}, {100, 0}, {80, 40}, {80, 80}, ArcLineIntersectExp {{80, 60}, 0.9273F}},
				{{310002000, 490203000}, {310202000, 490203000}, {310002000, 490003000}, {310200000, 490150000},
						{310150000, 490150000}, ArcLineIntersectExp {{310194850, 490150000}, 1.839F}},
		}),
		centre, arcStart, arcEnd, lineStart, lineEnd, expected)
{
	const ArcSegment arc {centre, arcStart, arcEnd};
	BOOST_TEST_INFO(arc.first);
	BOOST_TEST_INFO(arc.second);
	BOOST_TEST_INFO(arc.length());

	const auto intersection = arc.crossesLineAt(lineStart, lineEnd);
	BOOST_REQUIRE_EQUAL(expected.has_value(), intersection.has_value());
	if (expected.has_value()) {
		BOOST_CHECK_EQUAL(expected->first, intersection->first);
		BOOST_CHECK_CLOSE(expected->second, intersection->second, 1.F);
	}
}

static_assert(pointLeftOfLine({1, 2}, {1, 1}, {2, 2}));
static_assert(pointLeftOfLine({2, 1}, {2, 2}, {1, 1}));
static_assert(pointLeftOfLine({2, 2}, {1, 2}, {2, 1}));
static_assert(pointLeftOfLine({1, 1}, {2, 1}, {1, 2}));
static_assert(pointLeftOfOrOnLine({310000000, 490000000}, {310000000, 490000000}, {310050000, 490050000}));
static_assert(pointLeftOfOrOnLine({310000000, 490000000}, {310050000, 490050000}, {310000000, 490050000}));
static_assert(pointLeftOfOrOnLine({310000000, 490000000}, {310000000, 490050000}, {310000000, 490000000}));

static_assert(linesCross({1, 1}, {2, 2}, {1, 2}, {2, 1}));
static_assert(linesCross({2, 2}, {1, 1}, {1, 2}, {2, 1}));

static_assert(linesCrossLtR({1, 1}, {2, 2}, {1, 2}, {2, 1}));
static_assert(!linesCrossLtR({2, 2}, {1, 1}, {1, 2}, {2, 1}));

static_assert(Triangle<3, GlobalDistance> {{1, 2, 3}, {1, 0, 1}, {-2, 1, 0}}.positionOnPlane({7, -2})
		== GlobalPosition3D {7, -2, 3});
static_assert(Triangle<3, GlobalDistance> {
					  {310000000, 490000000, 32800}, {310050000, 490050000, 33000}, {310000000, 490050000, 32700}}
					  .positionOnPlane({310000000, 490000000})
		== GlobalPosition3D {310000000, 490000000, 32800});
static_assert(Triangle<3, GlobalDistance> {
					  {310750000, 490150000, 58400}, {310800000, 490200000, 55500}, {310750000, 490200000, 57600}}
					  .positionOnPlane({310751000, 490152000})
		== GlobalPosition3D {310751000, 490152000, 58326});
