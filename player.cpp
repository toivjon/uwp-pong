#include "player.h"

using namespace pong;

constexpr auto DEFAULT_NAME = L"player";

Player::Player() : mScore(0), mName(DEFAULT_NAME) {
	
}

void Player::ResetName() {
	mName = DEFAULT_NAME;
}