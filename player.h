#pragma once

namespace pong
{
	class Context;
	class Player final
	{
	public:
		void SetGamepad(Windows::Gaming::Input::Gamepad^ gamepad);
		
		void CheckInput(Context& ctx);

		Windows::Gaming::Input::Gamepad^ GetGamepad() { return mGamepad; }
		bool HasGamepad() const { return mGamepad != nullptr; }
	private:
		// The gamepad controller reference for the target player.
		Windows::Gaming::Input::Gamepad^ mGamepad;
	};
}
