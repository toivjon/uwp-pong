#pragma once

#include "audio.h"
#include "context.h"
#include "graphics.h"

#include <memory>

namespace pong
{
	// The base core class for the UWP game implementation.
	ref class Game sealed : public  Windows::ApplicationModel::Core::IFrameworkView, Windows::ApplicationModel::Core::IFrameworkViewSource
	{
	public:
		// The factory implementation required by the IFrameworkView interface.
		virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView();

		// The application initialization called by the target OS when application is launched.
		virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^);

		// The main window assignment called by the OS during application launch.
		virtual void SetWindow(Windows::UI::Core::CoreWindow^);

		// The function used to load resources at startup or restore from a sleeping state.
		virtual void Load(Platform::String^);

		// The function that starts and holds the actual processing logic.
		virtual void Run();

		// The function used to dispose reserved resources.
		virtual void Uninitialize();

		// The function that is called when the main window of the application is being closed.
		void OnWindowClosed(Windows::UI::Core::CoreWindow^, Windows::UI::Core::CoreWindowEventArgs^);

		// The function that is called when the application view gets activated.
		void OnActivated(Windows::ApplicationModel::Core::CoreApplicationView^, Windows::ApplicationModel::Activation::IActivatedEventArgs^);

		// The function that is called when window visibility is being changed.
		void OnWindowVisibilityChanged(Windows::UI::Core::CoreWindow^, Windows::UI::Core::VisibilityChangedEventArgs^);

		// The function that is called when a gamepad is being detected by the system.
		void OnGamepadAdded(Object^ o, Windows::Gaming::Input::Gamepad^ gamepad);

		// The function that is called when a gamepad is being removed.
		void OnGamepadRemoved(Object^ o, Windows::Gaming::Input::Gamepad^ gamepad);
	private:
		// The flag used to stop execution of the application's main loop when the main window is closed.
		bool mWindowClosed;

		// The flag used to indicate whether the main window is visible and game should be rendered.
		bool mWindowVisible;

		// The game context tracking the current state of the game.
		std::unique_ptr<Context> mContext;

		// The audio engine used to process sound effects.
		std::unique_ptr<Audio> mAudio;

		// The graphics engine used to process graphics.
		std::unique_ptr<Graphics> mGraphics;
	};
}
