#pragma once

#include "assetFactory/assetFactory.h"
#include <collection.h>
#include <memory>
#include <special_members.h>

class WorldObject;
class Terrain;
class Environment;
class Renderable;

class GameState {
public:
	GameState();
	~GameState();
	NO_MOVE(GameState);
	NO_COPY(GameState);

	SharedCollection<WorldObject, Renderable> world;
	std::shared_ptr<Terrain> terrain;
	std::shared_ptr<Environment> environment;
	AssetFactory::Assets assets;
};

extern GameState * gameState;
