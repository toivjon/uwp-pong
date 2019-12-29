#include "pch.h"

using namespace pong;
using namespace winrt;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Devices::Input;
using namespace Windows::Gaming::Input;
using namespace Windows::System;
using namespace Windows::UI::Core;

struct Pong : implements<Pong, IFrameworkViewSource, IFrameworkView>
{
public:
	// ========================================================================
	// A factory to build the view.
	IFrameworkView CreateView()
	{
		return *this;
	}

	// ========================================================================
	// Perform the initialization of the application view.
	void Initialize(CoreApplicationView const& application)
	{
		// activate main window when the application is activated.
		application.Activated([this](auto&&, auto&&) {
			CoreWindow::GetForCurrentThread().Activate();
		});

		// add gamepad into gamepad list when attached.
		Gamepad::GamepadAdded([this](auto&&, auto&& gamepad) {
			mGamepads.insert(gamepad);
		});

		// remove gamepad from gamepad list when detached.
		Gamepad::GamepadRemoved([this](auto&&, auto&& gamepad) {
			mGamepads.erase(gamepad);
		});
	}

	// ========================================================================
	// Assign the core window for the application view.
	void SetWindow(CoreWindow const& window)
	{
		// stop application execution when window closes.
		window.Closed([this](auto&&, auto&&) {
			mExiting = false;
		});

		// stop rendering when the main window is not visible.
		window.VisibilityChanged([this](auto&&, auto& args) {
			mVisible = args.Visible();
		});
	}

	// ========================================================================
	// Load the resources required to run the application.
	void Load(hstring const&)
	{
		// load the welcome scene as the initial scene.
		mScene = std::make_unique<WelcomeScene>();
		mScene->OnEnter();
	}

	// ========================================================================
	// Run the application until the application is exited.
	void Run()
	{
		mPreviousMillis = 0u;
		mDeltaAccumulator = 0u;
		while (!mExiting) {
			// calculate the delta time and store the current millis.
			auto currentMillis = GetCurrentMillis();
			auto dt = (currentMillis - mPreviousMillis);
			mPreviousMillis = currentMillis;

			auto window = CoreWindow::GetForCurrentThread();
			auto dispatcher = window.Dispatcher();
			if (mVisible) {
				for (auto& gamepad : mGamepads) {
					GamepadReading reading = gamepad.GetCurrentReading();
					GamepadButtons buttons = reading.Buttons;
					if ((buttons & GamepadButtons::A) != (GamepadButtons)0) {
						OutputDebugString(L"BOOM! A button was pressed!\n");
					}
				}
				if ((window.GetKeyState(VirtualKey::A) & CoreVirtualKeyStates::Down) != (CoreVirtualKeyStates)0) {
					OutputDebugString(L"BOOM! A key was pressed!\n");
				}

				// update game logics with a fixed framerate.
				mDeltaAccumulator += dt;
				static const uint64_t TIMESTEP = (1000 / 60);
				while (mDeltaAccumulator >= TIMESTEP) {
					mScene->OnUpdate();
					mDeltaAccumulator -= TIMESTEP;
				}

				// calculate interpolation alpha and render the scene.
				mRenderingCtx.SetAlpha((float)mDeltaAccumulator / (float)TIMESTEP);
				mScene->OnRender(mRenderingCtx);
				dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
			} else {
				dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
			}
		}
	}

	// ========================================================================
	// Release resource acquired for the application.
	// NOTE: This function is not always called, so we don't rely on it.
	void Uninitialize()
	{
		// ... something to do?
	}

	// ========================================================================
	// Get the current time in milliseconds.
	uint64_t GetCurrentMillis() {
		using namespace std::chrono;
		return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	}
private:
	bool					mExiting;
	bool					mVisible;
	std::set<Gamepad>		mGamepads;
	std::unique_ptr<Scene>	mScene;
	uint64_t				mPreviousMillis;
	uint64_t				mDeltaAccumulator;
	RenderingCtx			mRenderingCtx;
};

// ============================================================================
// The entry point of the application.
int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    CoreApplication::Run(make<Pong>());
}
