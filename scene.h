#pragma once

#include <memory>
#include <unordered_map>

namespace pong
{
	class Context;
	class Scene
	{
	public:
		enum class ID { MainMenu, Court, GameOver };

		static std::shared_ptr<Scene> Create(Scene::ID);

		virtual void OnEnter(Context& ctx);
		virtual void OnExit(Context& ctx);
		virtual void OnRender(Context& ctx);
		virtual void OnUpdate(Context& ctx);
	};
}
