#include "pch.hpp"
#include "audio.hpp"
#include "renderer.hpp"
#include "game.hpp"

using namespace std::chrono;
using namespace winrt;

using namespace concurrency;
using namespace Windows;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::Gaming::Input;
using namespace Windows::Graphics::Display;
using namespace Windows::System;
using namespace Windows::UI;
using namespace Windows::UI::Core;

struct App : implements<App, IFrameworkViewSource, IFrameworkView> {

	IFrameworkView CreateView() {
		return *this;
	}

	void Initialize(const CoreApplicationView& view) {
		OutputDebugStringA("App::Initialize\n");
		view.Activated({ this, &App::OnActivated });
		CoreApplication::EnteredBackground({ this, &App::OnEnteredBackground });
		CoreApplication::LeavingBackground({ this, &App::OnLeavingBackground });
		Gamepad::GamepadAdded({ this, &App::OnGamepadAdded });
		Gamepad::GamepadRemoved({ this, &App::OnGamepadRemoved });
		mRenderer = std::make_unique<Renderer>();
		mAudio = std::make_unique<Audio>();
		mGame = std::make_unique<Game>(mAudio);
	}

	void OnActivated(const CoreApplicationView&, const IActivatedEventArgs&) {
		OutputDebugStringA("App::OnActivated\n");
		auto window = CoreWindow::GetForCurrentThread();
		window.Activate();
		mRenderer->setWindow(window);
	}

	void OnEnteredBackground(const IInspectable&, const EnteredBackgroundEventArgs&) {
		OutputDebugStringA("App::OnEnteredBackground\n");
		mForeground = false;
	}

	void OnLeavingBackground(const IInspectable&, const LeavingBackgroundEventArgs&) {
		OutputDebugStringA("App::OnLeavingBackground\n");
		mForeground = true;
	}

	void Load(const hstring&) {
		// nothing...
	}

	void Uninitialize() {
		// nothing...
	}

	void Run() {
		auto previousTime = system_clock::now();
		while (true) {
			auto window = CoreWindow::GetForCurrentThread();
			auto dispatcher = window.Dispatcher();
			if (mForeground) {
				dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

				// Read gamepad readings.
				ReadGamepads();

				// Resolve the duration of the previous frame and ensure that we stay within reasonable limits.
				const static auto MaxFrameTime = 250ms;
				const auto currentTime = system_clock::now();
				auto frameTime = currentTime - previousTime;
				if (frameTime > MaxFrameTime) {
					frameTime = MaxFrameTime;
				}
				previousTime = currentTime;

				// Update game world by the amount of time passed and render the game scene.
				mGame->update(duration_cast<milliseconds>(frameTime));
				mRenderer->clear();
				mGame->render(mRenderer);
				mRenderer->present();
			} else {
				dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
			}
		}
	}

	void SetWindow(const CoreWindow& window) {
		OutputDebugStringA("App::SetWindow\n");
		window.SizeChanged({ this, &App::OnWindowSizeChanged });
		window.KeyDown({ this, &App::OnKeyDown });
		window.KeyUp({ this, &App::OnKeyUp });
		DisplayInformation::DisplayContentsInvalidated({ this, &App::OnDisplayContentsInvalidated });
		auto displayInfo = DisplayInformation::GetForCurrentView();
		displayInfo.DpiChanged({ this, &App::OnDPIChanged });
		mRenderer->setWindow(window);
	}

	void OnWindowSizeChanged(const CoreWindow& window, const WindowSizeChangedEventArgs&) {
		OutputDebugStringA("App::OnWindowSizeChanged\n");
		mRenderer->setWindowSize(Size(window.Bounds().Width, window.Bounds().Height));
	}

	void OnDisplayContentsInvalidated(const DisplayInformation&, const IInspectable&) {
		OutputDebugStringA("App::OnDisplayContentsInvalidated\n");
		mRenderer->initDeviceResources();
		mRenderer->initWindowResources();
	}

	void OnDPIChanged(const DisplayInformation& info, const IInspectable&) {
		OutputDebugStringA("App::OnDPIChanged\n");
		mRenderer->setDpi(info.LogicalDpi());
	}

	void OnKeyDown(const CoreWindow&, const KeyEventArgs& args) {
		OutputDebugStringA("App::OnKeyDown\n");
		mGame->onKeyDown(args);
	}

	void OnKeyUp(const CoreWindow&, const KeyEventArgs& args) {
		OutputDebugStringA("App::OnKeyUp\n");
		mGame->onKeyUp(args);
	}

	void OnGamepadAdded(const IInspectable&, const Gamepad& gamepad) {
		OutputDebugStringA("App::OnGamepadAdded\n");
		static const auto MaxPlayers = 2;
		critical_section::scoped_lock lock{ mGamepadLock };
		if (mGamepads.size() < MaxPlayers) {
			auto finder = std::find(begin(mGamepads), end(mGamepads), gamepad);
			if (finder == end(mGamepads)) {
				mGamepads.push_back(gamepad);
			}
		}
	}

	void OnGamepadRemoved(const IInspectable&, const Gamepad& gamepad) {
		OutputDebugStringA("App::OnGamepadRemoved\n");
		critical_section::scoped_lock lock{ mGamepadLock };
		auto finder = std::find(begin(mGamepads), end(mGamepads), gamepad);
		if (finder != end(mGamepads)) {
			mGamepads.erase(finder);
		}
	}

	void ReadGamepads() {
		critical_section::scoped_lock lock{ mGamepadLock };
		for (auto i = 0; i < mGamepads.size(); i++) {
			auto reading = mGamepads[i].GetCurrentReading();
			mGame->onReadGamepad(i, reading);
		}
	}

private:
	Renderer::Ptr        mRenderer;
	Audio::Ptr           mAudio;
	bool                 mForeground = false;
	Game::Ptr            mGame;
	critical_section     mGamepadLock;
	std::vector<Gamepad> mGamepads;
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
	CoreApplication::Run(make<App>());
}
