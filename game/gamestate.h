#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <collection.hpp>
#include <special_members.hpp>

class WorldObject;

class GameState {
public:
	GameState();
	~GameState();
	NO_MOVE(GameState);
	NO_COPY(GameState);

	Collection<WorldObject> world;
};
extern GameState * gameState;

#endif
