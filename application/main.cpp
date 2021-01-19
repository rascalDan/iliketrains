#include "gfx/window.h"
#include <SDL2/SDL.h>
#include <chrono>
#include <cmath>
#include <collection.hpp>
#include <game/physical.h>
#include <game/world.h>
#include <game/worldobject.h>
#include <gfx/gl/camera.h>
#include <gfx/gl/shader.h>
#include <gfx/gl/transform.h>
#include <glm/glm.hpp>
#include <memory>
#include <numbers>
#include <special_members.hpp>
#include <worker.h>

static const int DISPLAY_WIDTH = 800;
static const int DISPLAY_HEIGHT = 600;

class Monkey : public WorldObject, public Physical {
public:
	Monkey() : Physical {{}, "res/monkey3.obj", "res/bricks.jpg"} { }

	void
	tick(TickDuration elapsed) override
	{
		counter += 0.000625F * elapsed.count();
		location.GetRot().y = std::numbers::pi_v<double> + sin(counter);
		location.GetRot().z = 0.3 * cos(counter * 10);
	}

private:
	float counter {};
};

class SDL_Application {
public:
	SDL_Application()
	{
		SDL_Init(SDL_INIT_EVERYTHING);

		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	}
	~SDL_Application()
	{
		SDL_Quit();
	}
	NO_COPY(SDL_Application);
	NO_MOVE(SDL_Application);

	void
	run()
	{
		Collection<Window> windows;
		windows.create(DISPLAY_WIDTH, DISPLAY_HEIGHT, "OpenGL");

		Worker w;

		World world;
		world.create<Monkey>();

		Shader shader("./res/basicShader");
		Camera camera({0.0F, 0.0F, -5.0F}, 70.0F, (float)DISPLAY_WIDTH / (float)DISPLAY_HEIGHT, 0.1F, 100.0F);

		SDL_Event e;
		bool isRunning = true;

		auto t_start = std::chrono::high_resolution_clock::now();
		const auto framelen = std::chrono::milliseconds {1000} / 120;

		while (isRunning) {
			while (SDL_PollEvent(&e)) {
				if (e.type == SDL_QUIT) {
					isRunning = false;
				}
			}

			const auto t_end = std::chrono::high_resolution_clock::now();
			const auto t_passed = std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start);

			world.apply(&WorldObject::tick, t_passed);
			windows.apply(&Window::Clear, 0.0F, 0.0F, 0.0F, 1.0F);
			shader.Bind();
			world.apply<Physical>(&Physical::render, shader, camera);
			windows.apply(&Window::SwapBuffers);

			if (const auto snz = framelen - t_passed; snz.count() > 0) {
				SDL_Delay(snz.count());
			}
			t_start = t_end;
		}
	}
};

int
main(int, char **)
{
	SDL_Application().run();

	return 0;
}
