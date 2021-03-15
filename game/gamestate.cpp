#include "gamestate.h"
#include <cassert>

GameState * gameState {nullptr};

GameState::GameState()
{
	assert(!gameState);
	gameState = this;
}

GameState::~GameState()
{
	gameState = nullptr;
}
