#include "player.h"

using namespace pong;
using namespace Windows::Gaming::Input;

void Player::SetGamepad(Gamepad^ gamepad)
{
	mGamepad = gamepad;
}

void Player::CheckInput(Context& ctx)
{
	// TODO ...
}