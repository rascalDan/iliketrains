#define BOOST_TEST_MODULE UI
#include <boost/test/unit_test.hpp>
#include <stream_support.h>

#include "testHelpers.h"
#include "testMainWindow.h"
#include <gfx/models/texture.h>
#include <resource.h>
#include <ui/svgIcon.h>

constexpr GLsizei RENDER_SIZE = 64;

BOOST_GLOBAL_FIXTURE(TestMainWindowAppBase);

BOOST_AUTO_TEST_CASE(LoadFromFile)
{
	SvgIcon svg(ImageDimensions {RENDER_SIZE}, Resource::mapPath("ui/icon/rails.svg"));
	const auto size = Texture::getSize(svg.texture);
	BOOST_CHECK_EQUAL(size, TextureDimensions(RENDER_SIZE, RENDER_SIZE, 1));
	Texture::save(svg.texture, (ANALYSIS_DIRECTORY / "rails.tga").c_str());
}
