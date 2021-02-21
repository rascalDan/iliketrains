#define BOOST_TEST_MODULE test_maths

#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <stream_support.hpp>

#include <glm/glm.hpp>
#include <maths.h>
#include <tuple>
constexpr auto quarter_pi = pi / 4.F;

using vecter_to_angle = std::tuple<glm::vec3, float>;
BOOST_DATA_TEST_CASE(test_vector_yaw,
		boost::unit_test::data::make<vecter_to_angle>(
				{{north, 0}, {south, pi}, {west, half_pi}, {east, -half_pi}, {north + east, -quarter_pi},
						{south + east, quarter_pi * -3}, {north + west, quarter_pi}, {south + west, quarter_pi * 3}}),
		v, a)
{
	BOOST_CHECK_CLOSE(vector_yaw(v), a, 1.F);
}

BOOST_DATA_TEST_CASE(test_vector_pitch,
		boost::unit_test::data::make<vecter_to_angle>({
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

using normalize_angle = std::tuple<float, float>;
BOOST_DATA_TEST_CASE(test_angle_normalize,
		boost::unit_test::data::make<normalize_angle>({
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

using pos3_to_arc = std::tuple<glm::vec3, glm::vec3, glm::vec3, Arc>;
BOOST_DATA_TEST_CASE(test_create_arc,
		boost::unit_test::data::make<pos3_to_arc>({
				{{0, 0, 0}, north, east, {0, half_pi * 3}},
				{{0, 0, 0}, west, east, {half_pi, half_pi * 3}},
				{{0, 0, 0}, south, east, {pi, half_pi * 3}},
				{{0, 0, 0}, east, north, {-half_pi, 0}},
				{{0, 0, 0}, south, north, {pi, two_pi}},
				{{0, 0, 0}, east, south, {-half_pi, pi}},
		}),
		c, s, e, a)
{
	const Arc arc {c, s, e};
	BOOST_REQUIRE_LT(arc.first, arc.second);
	BOOST_CHECK_CLOSE(arc.first, a.first, 1.F);
	BOOST_CHECK_CLOSE(arc.second, a.second, 1.F);
}
