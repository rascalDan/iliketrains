#pragma once

#include "assetFactory/assetFactory.h"
#include <collection.h>
#include <memory>
#include <special_members.h>

class WorldObject;
class GeoData;
class Environment;

class GameState {
public:
	GameState();
	~GameState();
	NO_MOVE(GameState);
	NO_COPY(GameState);

	Collection<WorldObject> world;
	std::shared_ptr<GeoData> geoData;
	std::shared_ptr<Environment> environment;
	AssetFactory::Assets assets;
};

extern GameState * gameState;
