#define BOOST_TEST_MODULE test_text

#include "test-helpers.hpp"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <stream_support.hpp>

#include <ui/font.h>

struct FontTest : public Font {
	FontTest() : Font {"/usr/share/fonts/corefonts/arial.ttf", 48} { }
};

BOOST_TEST_DONT_PRINT_LOG_VALUE(Font::CharData);

using TextureSizeTestData = std::tuple<unsigned, unsigned, unsigned>;
BOOST_DATA_TEST_CASE(fontTextureSize, boost::unit_test::data::make<unsigned>({2, 3, 10, 50, 250}), fontHeight)
{
	auto isPowerOfTwo = [](auto x) {
		return (x & (x - 1)) == 0;
	};
	const auto res = Font::getTextureSize(fontHeight);
	// Power of 2 dimensions...
	BOOST_CHECK(isPowerOfTwo(res.x));
	BOOST_CHECK(isPowerOfTwo(res.y));
	// No bigger than max texture size...
	BOOST_CHECK_LE(res.x, GL_MAX_TEXTURE_SIZE);
	BOOST_CHECK_LE(res.y, GL_MAX_TEXTURE_SIZE);
	// Big enough to hold the raster...
	BOOST_CHECK_GE(res.y, 8); // Sensible minimum
	BOOST_CHECK_GE(res.y, fontHeight);
	// Keep the requested size
	BOOST_CHECK_EQUAL(res.z, fontHeight);
}

BOOST_FIXTURE_TEST_SUITE(ft, FontTest);

BOOST_AUTO_TEST_CASE(initialize_chardata)
{
	BOOST_CHECK_GE(charsData.size(), 72);
	BOOST_CHECK_EQUAL(fontTextures.size(), 2);
}

using CharDataTest = std::tuple<char, Font::CharData>;
BOOST_DATA_TEST_CASE(initialize_chardata_A,
		boost::unit_test::data::make<CharDataTest>({
				{'A', {0, {34, 35}, {627, 0}, {-1, 35}, 32}},
				{'I', {0, {6, 35}, {862, 0}, {4, 35}, 13}},
				{'j', {0, {11, 45}, {1656, 0}, {-3, 35}, 11}},
				{'o', {0, {24, 27}, {1748, 0}, {1, 27}, 27}},
		}),
		character, expected)
{
	const auto & cd = charsData.at(character);
	BOOST_CHECK_EQUAL(cd.textureIdx, expected.textureIdx);
	BOOST_CHECK_EQUAL(cd.size, expected.size);
	BOOST_CHECK_EQUAL(cd.position, expected.position);
	BOOST_CHECK_EQUAL(cd.bearing, expected.bearing);
	BOOST_CHECK_EQUAL(cd.advance, expected.advance);
}

static_assert(glm::vec2 {862, 0} / glm::vec2 {2048, 64} == glm::vec2 {0.4208984375, 0});
static_assert(glm::vec2 {866, 35} / glm::vec2 {2048, 64} == glm::vec2 {0.4228515625, 0.546875});

BOOST_AUTO_TEST_CASE(render_text)
{
	constexpr std::string_view text {"I Like Trains"};
	const auto spaces = std::count_if(text.begin(), text.end(), isspace);
	const auto tqs = render(text);
	BOOST_REQUIRE_EQUAL(tqs.size(), 1);
	const auto & t1 = tqs.begin();
	BOOST_CHECK_EQUAL(t1->first, fontTextures.front().texture);
	const auto & v = t1->second;
	BOOST_CHECK_EQUAL(v.size(), text.size() - spaces);

	BOOST_TEST_CONTEXT(size) {
		// I
		BOOST_CHECK_CLOSE_VEC(v[0][0], glm::vec4(4, 0, 0.42, 0.54));
		BOOST_CHECK_CLOSE_VEC(v[0][1], glm::vec4(10, 0, 0.42, 0.54));
		BOOST_CHECK_CLOSE_VEC(v[0][2], glm::vec4(10, 35, 0.42, 0));
		BOOST_CHECK_CLOSE_VEC(v[0][3], glm::vec4(4, 35, 0.42, 0));
		// (space, no glyph)
		// L
		BOOST_CHECK_CLOSE_VEC(v[1][0], glm::vec4(32, 0, 0.42, 0.54));
		BOOST_CHECK_CLOSE_VEC(v[1][1], glm::vec4(54, 0, 0.42, 0.54));
		BOOST_CHECK_CLOSE_VEC(v[1][2], glm::vec4(54, 35, 0.42, 0));
		BOOST_CHECK_CLOSE_VEC(v[1][3], glm::vec4(32, 35, 0.42, 0));
	}
}

BOOST_AUTO_TEST_SUITE_END()
