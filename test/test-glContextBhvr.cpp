#define BOOST_TEST_MODULE test_glcontextbehaviour

#include "test-helpers.hpp"
#include <boost/test/unit_test.hpp>

#include <gfx/gl/sceneRenderer.h>
#include <lib/glArrays.h>
#include <ui/applicationBase.h>
#include <ui/window.h>

BOOST_GLOBAL_FIXTURE(ApplicationBase);

#define TEST_WINDOW_PARAMS __FILE__, 0, 0, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN
static void
CreateProgramTest()
{
	ProgramRef p;
	BOOST_REQUIRE(p);
}
BOOST_AUTO_TEST_CASE(windowContextThingsBehaviour1)
{
	BOOST_REQUIRE(!glCreateProgram); // Init not called yet
	{
		SDL_WindowPtr window {TEST_WINDOW_PARAMS};
		BOOST_REQUIRE(window);
		BOOST_REQUIRE(!glCreateProgram);
		BOOST_REQUIRE_NE(glewInit(), GLEW_OK); // No context yet
		{
			SDL_GLContextPtr context {window};
			BOOST_REQUIRE(context);
			BOOST_REQUIRE(!glCreateProgram);
			BOOST_REQUIRE_EQUAL(glewInit(), GLEW_OK);
			BOOST_REQUIRE(glCreateProgram);
			CreateProgramTest();
		} // Context destroyed
		BOOST_REQUIRE(glCreateProgram); // Functions still set
		BOOST_REQUIRE_THROW({ ProgramRef p; }, std::exception); // Get fails with no context
		{
			SDL_GLContextPtr context {window};
			BOOST_REQUIRE(context);
			CreateProgramTest();
		}
	}
	{
		SDL_WindowPtr window {TEST_WINDOW_PARAMS};
		BOOST_REQUIRE(window);
		SDL_GLContextPtr context {window};
		BOOST_REQUIRE(context);
		CreateProgramTest();
	}
}

BOOST_AUTO_TEST_CASE(windowContextThingsBehaviour2)
{
	SDL_WindowPtr window1 {TEST_WINDOW_PARAMS};
	BOOST_REQUIRE(window1);
	{
		SDL_WindowPtr window2 {TEST_WINDOW_PARAMS};
		BOOST_REQUIRE(window2);
		SDL_GLContextPtr context {window2};
		BOOST_REQUIRE(context);
		CreateProgramTest();
	}
	BOOST_REQUIRE_THROW({ ProgramRef p; }, std::exception); // Get fails with no context
}

BOOST_AUTO_TEST_CASE(windowContextThingsBehaviour3)
{
	std::optional<SDL_WindowPtr> window1 {std::in_place, TEST_WINDOW_PARAMS};
	std::optional<SDL_WindowPtr> window2 {std::in_place, TEST_WINDOW_PARAMS};
	BOOST_REQUIRE(window1);
	BOOST_REQUIRE(window1.value());
	SDL_GLContextPtr context {window1.value()};
	BOOST_REQUIRE(context);
	CreateProgramTest();
	window1.reset();
	BOOST_REQUIRE_THROW({ ProgramRef p; }, std::exception); // Get fails with context's window gone
	window1.emplace(TEST_WINDOW_PARAMS);
	BOOST_REQUIRE(window1);
	BOOST_REQUIRE(window1.value());
	BOOST_REQUIRE_THROW({ ProgramRef p; }, std::exception); // Get still fails with context's window gone
}
