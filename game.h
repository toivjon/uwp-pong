#pragma once

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
	};
}
