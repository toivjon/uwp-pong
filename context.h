#pragma once

#include "audio.h"
#include "event.h"
#include "graphics.h"
#include "player.h"
#include "scene.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <queue>
#include <vector>

namespace pong
{
	class RenderingContext final
	{
	public:
		RenderingContext() = delete;
		RenderingContext(Graphics& graphics);

		const Graphics& GetGraphics() const { return mGraphics; }
			  Graphics& GetGraphics()		{ return mGraphics; }

		double GetAlpha() const		{ return mAlpha;	}
		void SetAlpha(double alpha) { mAlpha = alpha;	}
	private:
		double		mAlpha;
		Graphics&	mGraphics;
	};

	class Context final
	{
	public:
		Context();

		// The function used to perform the next logical step for the game.
		void Update(unsigned long dt);

		// The function used to render and present the contents of the current scene.
		void Render(double alpha);

		// The function used to change the current scene.
		void ChangeScene(const std::string& name);

		void EnqueueEvent(const Event& event);

		void SetWindow(Windows::UI::Core::CoreWindow^ window);

		void CheckInput();

		Player& GetLeftPlayer() 	{ return mLeftPlayer;	}
		Player& GetRightPlayer()	{ return mRightPlayer;  }
	private:
		// A priority queue to hold events for the next update iteration.
		std::priority_queue<Event> mEvents;

		// A reference to the currently active scene.
		std::shared_ptr<Scene> mScene;

		// A mapping containing all available scenes by their name.
		std::unordered_map<std::string, std::shared_ptr<Scene>> mScenes;

		Player mLeftPlayer;
		Player mRightPlayer;

		std::unique_ptr<Audio> mAudio;
		std::unique_ptr<Graphics> mGraphics;
	};
}
