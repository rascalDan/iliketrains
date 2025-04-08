#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl2.h>
#include <game/gamestate.h>
#include <gfx/gl/sceneProvider.h>
#include <gfx/gl/sceneRenderer.h>
#include <ui/applicationBase.h>
#include <ui/mainApplication.h>
#include <ui/mainWindow.h>

constexpr ScreenAbsCoord DEFAULT_WINDOW_SIZE {800, 600};

int
main(int argc, char ** argv)
{
	class ResViewer : GameState, MainApplication {
	public:
		void
		run(std::span<char * const> fileList)
		{
			windows.create<MainWindow>(DEFAULT_WINDOW_SIZE, "ILT - Resource Viewer");
			mainLoop();
		}
	};

	std::span files {argv, static_cast<size_t>(argc)};

	ResViewer {}.run(files.subspan(2));
}
