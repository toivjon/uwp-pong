#pragma once

#include <string>

namespace pong
{
	class Context final
	{
	public:
		// The function used to perform the next logical step for the game.
		void Update(unsigned long dt);

		// The function used to render and present the contents of the current scene.
		void Render(double alpha);

		// void ChangeScene(const std::string& name);
		// void PlaySound(const std::string& name);
	};
}
