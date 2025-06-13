#include "editNetwork.h"
#include "imgui_wrap.h"
#include <game/gamestate.h>
#include <game/terrain.h>
#include <gfx/gl/sceneShader.h>
#include <gfx/models/texture.h>

EditNetwork::EditNetwork(Network * n) : network {n}, snapPoints {network->getSnapPoints()} { }

bool
EditNetwork::click(const SDL_MouseButtonEvent & event, const Ray<GlobalPosition3D> & ray)
{
	switch (event.button) {
		case SDL_BUTTON_MIDDLE:
			currentStart.reset();
			candidates.clear();
			return true;
		case SDL_BUTTON_LEFT:
			if (!currentStart) {
				currentStart = resolveRay(ray);
			}
			else {
				if (const auto def = resolveRay(ray)) {
					candidates = network->create(
							gameState->terrain.get(), CreationDefinition {.fromEnd = *currentStart, .toEnd = *def});
					for (const auto & link : candidates) {
						network->add(gameState->terrain.get(), link);
					}
					if (continuousMode) {
						currentStart = def;
						currentStart->direction = candidates.back()->endAt(def->position)->dir;
					}
					else {
						currentStart.reset();
					}
					candidates.clear();
					snapPoints = network->getSnapPoints();
				}
			}
			return true;
		default:
			return false;
	}
}

bool
EditNetwork::move(const SDL_MouseMotionEvent &, const Ray<GlobalPosition3D> & ray)
{
	if (currentStart) {
		if (const auto def = resolveRay(ray)) {
			candidates = network->create(
					gameState->terrain.get(), CreationDefinition {.fromEnd = *currentStart, .toEnd = *def});
		}
	}
	return false;
}

bool
EditNetwork::handleInput(const SDL_Event &)
{
	return false;
}

void
EditNetwork::render(const SceneShader &, const Frustum &) const
{
	// TODO render snapPoints
}

std::optional<CreationDefinitionEnd>
EditNetwork::resolveRay(const Ray<GlobalPosition3D> & ray) const
{
	if (const auto position = gameState->terrain->intersectRay(ray)) {
		if (const auto closestSnapPoint = std::ranges::min_element(snapPoints, {},
					[position](auto && snapPoint) {
						return distance(position->first, snapPoint.snapPosition);
					});
				closestSnapPoint != snapPoints.end()
				&& distance(position->first, closestSnapPoint->snapPosition) < 1'200.F) {
			return *closestSnapPoint;
		}
		return CreationDefinitionEnd {.position = position->first, .direction = std::nullopt};
	}
	return {};
}

void
EditNetwork::render(bool & open)
{
	ImGui::SetNextWindowSize({-1, -1});
	ImGui::Begin("Edit Network", &open);
	ImGui::Checkbox("Continuous mode", &continuousMode);
	ImGui::End();
}
