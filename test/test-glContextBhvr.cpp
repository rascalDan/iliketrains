#define BOOST_TEST_MODULE test_glcontextbehaviour
#include <boost/test/unit_test.hpp>

#include <gfx/gl/sceneRenderer.h>
#include <lib/glArrays.h>
#include <optional>
#include <ui/applicationBase.h>
#include <ui/window.h>

BOOST_GLOBAL_FIXTURE(ApplicationBase);

#define TEST_WINDOW_PARAMS __FILE__, 0, 0, 640, 480, static_cast<Uint32>(SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN)

namespace {
	void
	createProgramTest()
	{
		const ProgramRef prog;
		BOOST_REQUIRE(prog);
	}
}

BOOST_AUTO_TEST_CASE(WindowContextThingsBehaviour1)
{
	BOOST_REQUIRE(!glCreateProgram); // Init not called yet
	{
		const SDL_WindowPtr window {TEST_WINDOW_PARAMS};
		BOOST_REQUIRE(window);
		BOOST_REQUIRE(!glCreateProgram);
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		BOOST_REQUIRE_EQUAL(gladLoadGL(reinterpret_cast<GLADloadfunc>(SDL_GL_GetProcAddress)), 0); // No context yet
		{
			const SDL_GLContextPtr context {window};
			BOOST_REQUIRE(context);
			BOOST_REQUIRE(!glCreateProgram);
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			BOOST_REQUIRE_GT(gladLoadGL(reinterpret_cast<GLADloadfunc>(SDL_GL_GetProcAddress)), 0);
			BOOST_REQUIRE(glCreateProgram);
			createProgramTest();
		} // Context destroyed
		BOOST_REQUIRE(glCreateProgram); // Functions still set
		BOOST_REQUIRE_THROW({ const ProgramRef prog; }, std::exception); // Get fails with no context
		{
			const SDL_GLContextPtr context {window};
			BOOST_REQUIRE(context);
			createProgramTest();
		}
	}
	{
		const SDL_WindowPtr window {TEST_WINDOW_PARAMS};
		BOOST_REQUIRE(window);
		const SDL_GLContextPtr context {window};
		BOOST_REQUIRE(context);
		createProgramTest();
	}
}

BOOST_AUTO_TEST_CASE(WindowContextThingsBehaviour2)
{
	const SDL_WindowPtr window1 {TEST_WINDOW_PARAMS};
	BOOST_REQUIRE(window1);
	{
		const SDL_WindowPtr window2 {TEST_WINDOW_PARAMS};
		BOOST_REQUIRE(window2);
		const SDL_GLContextPtr context {window2};
		BOOST_REQUIRE(context);
		createProgramTest();
	}
	BOOST_REQUIRE_THROW({ const ProgramRef prog; }, std::exception); // Get fails with no context
}

BOOST_AUTO_TEST_CASE(WindowContextThingsBehaviour3)
{
	std::optional<SDL_WindowPtr> window1 {std::in_place, TEST_WINDOW_PARAMS};
	const std::optional<SDL_WindowPtr> window2 {std::in_place, TEST_WINDOW_PARAMS};
	BOOST_REQUIRE(window1);
	BOOST_REQUIRE(window1.value());
	const SDL_GLContextPtr context {window1.value()};
	BOOST_REQUIRE(context);
	createProgramTest();
	window1.reset();
	BOOST_REQUIRE_THROW({ const ProgramRef prog; }, std::exception); // Get fails with context's window gone
	window1.emplace(TEST_WINDOW_PARAMS);
	BOOST_REQUIRE(window1);
	BOOST_REQUIRE(window1.value());
	BOOST_REQUIRE_THROW({ const ProgramRef prog; }, std::exception); // Get still fails with context's window gone
}
