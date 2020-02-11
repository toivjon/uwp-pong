#pragma once

#include "event.h"

#include <string>
#include <queue>
#include <vector>

namespace pong
{
	class Context final
	{
	public:
		// The function used to perform the next logical step for the game.
		void Update(unsigned long dt);

		// The function used to render and present the contents of the current scene.
		void Render(double alpha);

		// The function used to change the current scene.
		void ChangeScene(const std::string& name);
	private:
		// A priority queue to hold events for the next update iteration.
		std::priority_queue<Event> mEvents;
	};
}
