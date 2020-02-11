#pragma once

namespace pong
{
	class Context;
	class Scene final
	{
	public:
		void OnEnter(Context& ctx);
		void OnExit(Context& ctx);
		void OnRender(Context& ctx);
		void OnUpdate(Context& ctx);
	};
}
