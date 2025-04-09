#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl2.h>
#include <game/environment.h>
#include <game/gamestate.h>
#include <game/terrain.h>
#include <gfx/gl/sceneProvider.h>
#include <gfx/gl/sceneRenderer.h>
#include <gfx/renderable.h>
#include <location.h>
#include <ui/applicationBase.h>
#include <ui/mainApplication.h>
#include <ui/mainWindow.h>

constexpr ScreenAbsCoord DEFAULT_WINDOW_SIZE {800, 600};
constexpr GlobalDistance TERRAIN_LIMIT = 1'000'000;
constexpr GlobalDistance TERRAIN_HEIGHT = 10'000;
constexpr RelativeDistance DEFAULT_CAMERA_DIST = 7'000;
constexpr GlobalDistance DEFAULT_CAMERA_HEIGHT = 5'000;
constexpr GlobalDistance DEFAULT_CAMERA_FOCUS = 3'000;
constexpr GlobalDistance MAX_CAMERA_HEIGHT = 10'000;
constexpr GlobalDistance MIN_CAMERA_DIST = 1'000;
constexpr GlobalDistance MAX_CAMERA_DIST = 30'000;

class ViewerContent : public WindowContent, SceneRenderer, SceneProvider {
public:
	ViewerContent(ScreenAbsCoord size, std::span<char * const> files) : SceneRenderer {size, 0}, fileList(files)
	{
		camera.setPosition(calcCameraPosition());
		camera.lookAt({0, 0, TERRAIN_HEIGHT + cameraFocus});
		gameState->terrain = std::make_shared<Terrain>(
				GeoData::createFlat({-TERRAIN_LIMIT, -TERRAIN_LIMIT}, {TERRAIN_LIMIT, TERRAIN_LIMIT}, TERRAIN_HEIGHT));
	}

private:
	[[nodiscard]]
	GlobalPosition3D
	calcCameraPosition() const
	{
		return {sincos(cameraAngle) * cameraDistance, TERRAIN_HEIGHT + cameraHeight};
	}

	void
	render() override
	{
		SceneRenderer::render(*this);
		controls();
	}

	void
	tick(TickDuration tick) override
	{
		if (autoRotate != 0) {
			cameraAngle = normalize(cameraAngle + autoRotate * tick.count());
			camera.setPosition(calcCameraPosition());
			camera.lookAt({0, 0, TERRAIN_HEIGHT + cameraFocus});
		}
	}

	bool
	handleInput(const SDL_Event & event) override
	{
		if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
			SceneRenderer::resize({event.window.data1, event.window.data2});
		}

		return WindowContent::handleInput(event);
	}

	void
	controls()
	{
		if (ImGui::Begin("Resource view")) {
			ImGui::SetWindowSize({});
			fileSelection();
			assetSelection();
		}
		ImGui::End();

		if (ImGui::Begin("Camera")) {
			ImGui::SetWindowSize({});
			if (std::max({ImGui::SliderFloat("Camera position", &cameraAngle, -pi, pi),
						ImGui::SliderInt("Camera focus", &cameraFocus, 1, MAX_CAMERA_HEIGHT),
						ImGui::SliderInt("Camera height", &cameraHeight, 1, MAX_CAMERA_HEIGHT),
						ImGui::SliderFloat("Camera distance", &cameraDistance, MIN_CAMERA_DIST, MAX_CAMERA_DIST)})) {
				camera.setPosition(calcCameraPosition());
				camera.lookAt({0, 0, TERRAIN_HEIGHT + cameraFocus});
			}
			ImGui::SliderFloat("Auto rotate speed", &autoRotate, -1, 1);
		}
		ImGui::End();
	}

	void
	fileSelection()
	{
		ImGui::BeginListBox("File");
		for (const auto file : fileList) {
			if (ImGui::Selectable(file, file == selectedFile)) {
				location.reset();
				selectedAsset = nullptr;
				gameState->assets = AssetFactory::loadXML(file)->assets;
				selectedFile = file;
			}
		}
		ImGui::EndListBox();
	}

	void
	assetSelection()
	{
		if (!gameState->assets.empty()) {
			ImGui::BeginListBox("Asset");
			for (const auto & asset : gameState->assets) {
				auto renderable = asset.second.getAs<const Renderable>();
				if (renderable) {
					if (ImGui::Selectable(asset.first.c_str(), renderable == selectedAsset)) {
						selectedAsset = renderable;
						location = asset.second->createAt(position);
					}
				}
			}
			ImGui::EndListBox();
		}
	}

	void
	content(const SceneShader & sceneShader, const Frustum & frustum) const override
	{
		gameState->terrain->render(sceneShader, frustum);
		if (selectedAsset) {
			selectedAsset->render(sceneShader, frustum);
		}
	}

	void
	lights(const SceneShader & sceneShader) const override
	{
		if (selectedAsset) {
			selectedAsset->lights(sceneShader);
		}
	}

	void
	shadows(const ShadowMapper & mapper, const Frustum & frustum) const override
	{
		gameState->terrain->shadows(mapper, frustum);
		if (selectedAsset) {
			selectedAsset->shadows(mapper, frustum);
		}
	}

	std::span<char * const> fileList;
	char * selectedFile {};
	const Renderable * selectedAsset {};
	Location position {.pos = {0, 0, TERRAIN_HEIGHT}, .rot = {}};
	std::any location;
	Angle cameraAngle {0.F};
	RelativeDistance cameraDistance {DEFAULT_CAMERA_DIST};
	GlobalDistance cameraHeight {DEFAULT_CAMERA_HEIGHT};
	GlobalDistance cameraFocus {DEFAULT_CAMERA_FOCUS};
	float autoRotate {0.F};
};

int
main(int argc, char ** argv)
{
	class ResViewer : GameState, MainApplication {
	public:
		void
		run(std::span<char * const> fileList)
		{
			windows.create<MainWindow>(DEFAULT_WINDOW_SIZE, "ILT - Resource Viewer")
					->setContent<ViewerContent>(fileList);
			mainLoop();
		}
	};

	std::span files {argv, static_cast<size_t>(argc)};

	ResViewer {}.run(files.subspan(2));
}
