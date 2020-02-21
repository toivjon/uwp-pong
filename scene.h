#pragma once

#include <memory>

namespace pong
{
	class Context;
	class Graphics;
	class Scene
	{
	public:
		// An enumeration of all available scenes.
		enum class ID { MainMenu, Court, GameOver };

		// A cached factory function to build a scene.
		static std::shared_ptr<Scene> Get(Scene::ID);

		// A function called when the scene gets activated.
		virtual void OnEnter(Context& ctx) = 0;

		// A function called when the scene gets exited.
		virtual void OnExit(Context& ctx) = 0;

		// A function called on each logical game tick.
		virtual void OnUpdate(Context& ctx) = 0;
	};
}
