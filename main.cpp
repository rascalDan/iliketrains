#include "camera.h"
#include "display.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "transform.h"
#include <SDL2/SDL.h>
#include <cmath>
#include <glm/glm.hpp>
#include <memory>
#include <numbers>

static const int DISPLAY_WIDTH = 800;
static const int DISPLAY_HEIGHT = 600;

int
main(int, char **)
{
	Display display(DISPLAY_WIDTH, DISPLAY_HEIGHT, "OpenGL");

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

		display.Clear(0.0F, 0.0F, 0.0F, 1.0F);

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

		display.SwapBuffers();
		SDL_Delay(1);
		counter += 0.01F;
	}

	return 0;
}
