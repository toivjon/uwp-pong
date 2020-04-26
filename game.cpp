#include "game.h"

#include <cassert>

using namespace Pong;

void Game::Run() {
	assert(state == State::STOPPED);
	state = State::RUNNING;
	// ...
}

void Game::Stop() {
	state = State::STOPPED;
}

void Game::Pause() {
	state = State::PAUSED;
}

void Game::Resume() {
	state = State::RUNNING;
}

void Game::SetResolution(float width, float height) {

}