#pragma once

#include <collection.hpp>
#include <memory>
#include <special_members.hpp>

class WorldObject;
class GeoData;

class GameState {
public:
	GameState();
	~GameState();
	NO_MOVE(GameState);
	NO_COPY(GameState);

	Collection<WorldObject> world;
	std::shared_ptr<GeoData> geoData;
};
extern GameState * gameState;
