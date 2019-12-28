#pragma once

namespace pong
{
	// The base scene abstraction for all scenes within the application.
	class Scene
	{
	public:
		// A function which gets called on each logic tick.
		virtual void OnUpdate() = 0;
		// A function which gets called on each rendering ticket.
		virtual void OnRender() = 0;
		// A function which gets called when the scene is entered.
		virtual void OnEnter() = 0;
		// A function which gets called when the scene is being leaved.
		virtual void OnLeave() = 0;
	};
}