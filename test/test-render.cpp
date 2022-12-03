#define BOOST_TEST_MODULE test_render

#include "test-helpers.hpp"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <game/geoData.h>
#include <game/terrain.h>
#include <gfx/gl/sceneRenderer.h>
#include <gfx/models/texture.h>
#include <lib/glArrays.h>
#include <maths.h>
#include <ui/applicationBase.h>
#include <ui/window.h>

class TestRenderOutput {
public:
	TestRenderOutput() : size {640, 480}
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
	const glm::ivec2 size;
	glFrameBuffer output;
	glRenderBuffer depth;
	glTexture outImage;
};

class TestMainWindow : public Window {
	// This exists only to hold an OpenGL context open for the duration of the tests,
	// in the same way a real main window would always exist.
public:
	TestMainWindow() : Window {1, 1, __FILE__, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN}
	{
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(
				[](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message,
						const void *) {
					char buf[BUFSIZ];
					snprintf(buf, BUFSIZ, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s",
							(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
					switch (type) {
						case GL_DEBUG_TYPE_ERROR:
						case GL_DEBUG_TYPE_PERFORMANCE:
						case GL_DEBUG_TYPE_PORTABILITY:
						case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
						case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
							BOOST_TEST_ERROR(buf);
					}
					BOOST_TEST_MESSAGE(buf);
				},
				nullptr);
	}
	void
	tick(TickDuration) override
	{
	}
};

class TestScene : public SceneProvider {
	Terrain terrain {[]() {
		auto gd = std::make_shared<GeoData>(GeoData::Limits {{0, 0}, {100, 100}});
		gd->generateRandom();
		return gd;
	}()};
	void
	content(const SceneShader & shader) const
	{
		terrain.render(shader);
	}
	void
	lights(const SceneShader &) const
	{
	}
};

BOOST_GLOBAL_FIXTURE(ApplicationBase);
BOOST_GLOBAL_FIXTURE(TestMainWindow);

BOOST_FIXTURE_TEST_SUITE(w, TestRenderOutput);

BOOST_AUTO_TEST_CASE(basic)
{
	SceneRenderer ss {size, output};
	ss.camera.pos = {-10, -10, 60};
	ss.camera.forward = glm::normalize(glm::vec3 {1, 1, -0.5F});
	TestScene scene;
	ss.render(scene);
	Texture::save(outImage, size, "/tmp/basic.tga");
}

BOOST_AUTO_TEST_CASE(pointlight)
{
	SceneRenderer ss {size, output};
	ss.camera.pos = {-10, -10, 60};
	ss.camera.forward = glm::normalize(glm::vec3 {1, 1, -0.5F});
	class PointLightScene : public TestScene {
	public:
		void
		environment(const SceneShader &, const SceneRenderer & r) const override
		{
			r.setAmbientLight({0.2F, 0.2F, 0.2F});
			r.setDirectionalLight({}, down);
		}
		void
		lights(const SceneShader & shader) const override
		{
			for (int x = 50; x < 100; x += 20) {
				for (int y = 50; y < 2000; y += 20) {
					shader.pointLight.add({x, y, 4}, {1.0, 1.0, 1.0}, 0.1);
				}
			}
		}
	};
	PointLightScene scene;
	ss.render(scene);
	Texture::save(outImage, size, "/tmp/pointlight.tga");
}

BOOST_AUTO_TEST_SUITE_END();
