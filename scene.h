#pragma once

namespace pong
{
	class Scene
	{
	public:
		// function which gets called on each logic tick.
		virtual void OnUpdate() = 0;
		// function which gets called on each rendering ticket.
		virtual void OnRender() = 0;
		// function which gets called when the scene is entered.
		virtual void onEnter() = 0;
		// function which gets called when the scene is being leaved.
		virtual void OnLeave() = 0;
	};
}