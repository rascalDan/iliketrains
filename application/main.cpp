#include "gfx/window.h"
#include <SDL2/SDL.h>
#include <cmath>
#include <gfx/gl/camera.h>
#include <gfx/gl/shader.h>
#include <gfx/gl/transform.h>
#include <gfx/models/mesh.h>
#include <gfx/models/texture.h>
#include <glm/glm.hpp>
#include <memory>
#include <numbers>
#include <special_members.hpp>

static const int DISPLAY_WIDTH = 800;
static const int DISPLAY_HEIGHT = 600;

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
		Window main_window(DISPLAY_WIDTH, DISPLAY_HEIGHT, "OpenGL");

		Mesh monkey("./res/monkey3.obj");
		Shader shader("./res/basicShader");
		Texture texture("./res/bricks.jpg");
		Transform transform;
		Camera camera(glm::vec3(0.0F, 0.0F, -5.0F), 70.0F, (float)DISPLAY_WIDTH / (float)DISPLAY_HEIGHT, 0.1F, 100.0F);

		SDL_Event e;
		bool isRunning = true;
		float counter = 0.0F;
		while (isRunning) {
			while (SDL_PollEvent(&e)) {
				if (e.type == SDL_QUIT) {
					isRunning = false;
				}
			}

			main_window.Clear(0.0F, 0.0F, 0.0F, 1.0F);

			// float sinCounter = sinf(counter);
			// float absSinCounter = abs(sinCounter);

			// transform.GetPos()->x = sinCounter;
			transform.GetRot().y = std::numbers::pi_v<double> + sin(counter);
			transform.GetRot().z = 0.3 * cos(counter * 10);
			// transform.GetScale()->x = absSinCounter;
			// transform.GetScale()->y = absSinCounter;

			shader.Bind();
			texture.Bind();
			shader.Update(transform, camera);
			monkey.Draw();

			main_window.SwapBuffers();
			SDL_Delay(1);
			counter += 0.01F;
		}
	}
};

int
main(int, char **)
{
	SDL_Application().run();

	return 0;
}
