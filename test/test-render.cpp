#define BOOST_TEST_MODULE test_render

#include "test-helpers.hpp"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <game/geoData.h>
#include <game/terrain.h>
#include <gfx/models/texture.h>
#include <lib/glArrays.h>
#include <ui/applicationBase.h>
#include <ui/sceneRenderer.h>
#include <ui/window.h>

class TestRenderWindow : public Window {
public:
	TestRenderWindow() : Window(640, 480, __FILE__, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, output);
		const auto configuregdata
				= [this](const GLuint data, const GLint format, const GLenum type, const GLenum attachment) {
					  glBindTexture(GL_TEXTURE_2D, data);
					  glTexImage2D(GL_TEXTURE_2D, 0, format, size.x, size.y, 0, GL_RGBA, type, NULL);
					  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					  glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, data, 0);
				  };
		configuregdata(outImage, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		glBindRenderbuffer(GL_RENDERBUFFER, depth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, size.x, size.y);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);

		// finally check if framebuffer is complete
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			throw std::runtime_error("Framebuffer not complete!");
		}
	}
	void
	tick(TickDuration) override
	{
	}
	glFrameBuffer output;
	glRenderBuffer depth;
	glTexture outImage;
};

BOOST_GLOBAL_FIXTURE(ApplicationBase);

BOOST_FIXTURE_TEST_SUITE(w, TestRenderWindow);

BOOST_AUTO_TEST_CASE(basic)
{
	auto gd = std::make_shared<GeoData>(GeoData::Limits {{0, 0}, {100, 100}});
	gd->generateRandom();
	Terrain terrain {gd};
	SceneRenderer ss {size, output};
	ss.camera.pos = {-10, -10, 60};
	ss.camera.forward = glm::normalize(glm::vec3 {1, 1, -0.5F});
	ss.render([&terrain](const auto & shader) {
		terrain.render(shader);
	});
	Texture::save(outImage, size, "/tmp/basic.tga");
}

BOOST_AUTO_TEST_SUITE_END();
