#include "gamestate.h"
#include "environment.h"
#include <cassert>

GameState * gameState {nullptr};

GameState::GameState()
{
	assert(!gameState);
	gameState = this;

	environment = world.create<Environment>();
}

GameState::~GameState()
{
	gameState = nullptr;
}
