#include <SDL2/SDL.h>
#include <chrono>
#include <collection.hpp>
#include <game/terrain.h>
#include <game/world.h>
#include <game/worldobject.h>
#include <gfx/camera_controller.h>
#include <gfx/gl/camera.h>
#include <gfx/gl/shader.h>
#include <gfx/inputHandler.h>
#include <gfx/manualCameraController.h>
#include <gfx/renderable.h>
#include <gfx/window.h>
#include <memory>
#include <special_members.hpp>
#include <vector>
#include <worker.h>

static const int DISPLAY_WIDTH = 800;
static const int DISPLAY_HEIGHT = 600;

class SDL_Application : public InputHandler, public std::enable_shared_from_this<SDL_Application> {
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

	~SDL_Application() override
	{
		SDL_Quit();
	}

	NO_COPY(SDL_Application);
	NO_MOVE(SDL_Application);

	bool
	handleInput(SDL_Event & e) override
	{
		switch (e.type) {
			case SDL_QUIT:
				isRunning = false;
				return true;
		}
		return false;
	}

	int
	run()
	{
		Collection<Window> windows;
		windows.create(DISPLAY_WIDTH, DISPLAY_HEIGHT, "OpenGL");

		Worker w;

		world.create<Terrain>();

		Shader shader;
		Camera camera({-1250.0F, 35.0F, -1250.0F}, 70.0F, (float)DISPLAY_WIDTH / (float)DISPLAY_HEIGHT, 0.1F, 10000.0F);
		camera.Pitch(0.24);
		camera.RotateY(0.7854);
		shader.Bind();
		shader.setView(camera.GetViewProjection());

		auto t_start = std::chrono::high_resolution_clock::now();
		const auto framelen = std::chrono::milliseconds {1000} / 120;

		inputStack.objects.push_back(shared_from_this());
		inputStack.objects.insert(inputStack.objects.begin(), world.create<ManualCameraController>());

		while (isRunning) {
			processInputs();
			const auto t_end = std::chrono::high_resolution_clock::now();
			const auto t_passed = std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start);

			world.apply(&WorldObject::tick, t_passed);
			world.apply<CameraController>(&CameraController::updateCamera, &camera, &shader);
			windows.apply(&Window::Clear, 0.0F, 0.0F, 0.0F, 1.0F);
			world.apply<Renderable>(&Renderable::render, shader);
			windows.apply(&Window::SwapBuffers);

			if (const auto snz = framelen - t_passed; snz.count() > 0) {
				SDL_Delay(snz.count());
			}
			t_start = t_end;
		}

		return 0;
	}

private:
	void
	processInputs()
	{
		for (SDL_Event e; SDL_PollEvent(&e);) {
			inputStack.applyOne(&InputHandler::handleInput, e);
		}
	}

	bool isRunning {true};
	Collection<InputHandler> inputStack;
	World world;
};

int
main(int, char **)
{
	return std::make_shared<SDL_Application>()->run();
}
