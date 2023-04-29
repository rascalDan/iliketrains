#define BOOST_TEST_MODULE test_text

#include "testHelpers.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <stream_support.h>

#include <array>
#include <glm/glm.hpp>
#include <span>
#include <ui/font.h>
#include <unicode.h>
#include <vector>

BOOST_AUTO_TEST_CASE(utf8_string_view_iter)
{
	static constexpr utf8_string_view text {"Some UTF-8 €£²¹ text."};
	static constexpr std::array codepoints {
			83, 111, 109, 101, 32, 85, 84, 70, 45, 56, 32, 8364, 163, 178, 185, 32, 116, 101, 120, 116, 46};
	BOOST_CHECK_EQUAL(std::count_if(text.begin(), text.end(), isspace), 3);
	BOOST_CHECK_EQUAL(text.length(), 21);
	std::vector<uint32_t> codepointsOut;
	std::copy(text.begin(), text.end(), std::back_inserter(codepointsOut));
	BOOST_CHECK_EQUAL_COLLECTIONS(codepoints.begin(), codepoints.end(), codepointsOut.begin(), codepointsOut.end());
}

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

using CharDataTest = std::tuple<decltype(get_codepoint(nullptr)), Font::CharData>;
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
	const auto spaces = static_cast<std::size_t>(std::count_if(text.begin(), text.end(), isspace));
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

BOOST_AUTO_TEST_CASE(stream_vec)
{
	BOOST_CHECK_EQUAL(streamed_string(glm::vec3 {1.2, 2.3, 3.4}), "(1.2, 2.3, 3.4)");
}

BOOST_AUTO_TEST_CASE(stream_array)
{
	BOOST_CHECK_EQUAL(streamed_string(std::array {1.2, 2.3, 3.4}), "(1.2, 2.3, 3.4)");
}

BOOST_AUTO_TEST_CASE(stream_vector)
{
	BOOST_CHECK_EQUAL(streamed_string(std::vector {1.2, 2.3, 3.4}), "(1.2, 2.3, 3.4)");
}

BOOST_AUTO_TEST_CASE(stream_mat)
{
	BOOST_CHECK_EQUAL(streamed_string(glm::mat2 {1.2, 2.3, 3.4, 4.5}), "((1.2, 2.3), (3.4, 4.5))");
}
