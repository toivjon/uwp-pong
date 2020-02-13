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
		Windows::Gaming::Input::Gamepad^ mGamepad;
		Windows::Gaming::Input::GamepadReading mOldReading;
		Windows::Gaming::Input::GamepadReading mNewReading;
	};
}
