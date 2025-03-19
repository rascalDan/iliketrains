#include "queryTool.h"
#include "imgui_wrap.h"
#include <game/gamestate.h>
#include <game/selectable.h>
#include <game/terrain.h>
#include <game/worldobject.h>
#include <ray.h>
#include <stream_support.h>

bool
QueryTool::click(const SDL_MouseButtonEvent & event, const Ray<GlobalPosition3D> & ray)
{
	if (event.button != SDL_BUTTON_LEFT) {
		return false;
	}
	BaryPosition baryPos {};
	RelativeDistance distance {};

	if (const auto selected = gameState->world.applyOne<Selectable>(&Selectable::intersectRay, ray, baryPos, distance);
			selected != gameState->world.end()) {
		const auto & ref = *selected.base()->get();
		clicked = typeid(ref).name();
	}
	else if (const auto pos = gameState->terrain->intersectRay(ray)) {
		clicked = streamed_string(*pos);
	}
	else {
		clicked.clear();
	}
	return true;
}

void
QueryTool::render()
{
	ImGui::Begin("Query Tool");
	ImGui::TextUnformatted(clicked.c_str());
	ImGui::End();
}
