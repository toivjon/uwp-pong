#pragma once

#include <memory>

namespace pong
{
	class Context;
	class Scene
	{
	public:
		// An enumeration of all available scenes.
		enum class ID { MainMenu, Court, GameOver };

		// A cached factory function to build a scene.
		static std::shared_ptr<Scene> Get(Scene::ID);

		virtual void OnEnter(Context& ctx);
		virtual void OnExit(Context& ctx);
		virtual void OnRender(Context& ctx);
		virtual void OnUpdate(Context& ctx);
	};
}
