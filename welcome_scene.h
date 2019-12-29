#pragma once

#include "pch.h"

namespace pong
{
	// The first scene which is shown to players after startup.
	class WelcomeScene final : public Scene
	{
	public:
		void OnUpdate() override;
		void OnRender(RenderingCtx& renderer) override;
		void OnEnter() override;
		void OnLeave() override;
	};
}
