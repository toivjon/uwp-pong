#include "player.h"

using namespace pong;
using namespace Windows::Gaming::Input;

void Player::SetGamepad(Gamepad^ gamepad)
{
	mGamepad = gamepad;
	mNewReading = mGamepad->GetCurrentReading();
	mOldReading = mNewReading;
}

void Player::CheckInput(Context& ctx)
{
	if (mGamepad != nullptr) {
		mOldReading = mNewReading;
		mNewReading = mGamepad->GetCurrentReading();
	}
	// TODO ...
}