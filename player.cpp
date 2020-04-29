#include "player.h"

using namespace Pong;

constexpr auto DEFAULT_NAME = L"<< waiting for player >>";

Player::Player() : mScore(0), mName(DEFAULT_NAME) {
	
}

void Player::ResetName() {
	mName = DEFAULT_NAME;
}