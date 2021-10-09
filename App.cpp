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
		view.Activated({ this, &App::OnActivated });
		CoreApplication::EnteredBackground({ this, &App::OnEnteredBackground });
		CoreApplication::LeavingBackground({ this, &App::OnLeavingBackground });
		Gamepad::GamepadAdded({ this, &App::OnGamepadAdded });
		Gamepad::GamepadRemoved({ this, &App::OnGamepadRemoved });
		renderer = std::make_unique<Renderer>();
		audio = std::make_unique<Audio>();
		game = std::make_unique<Game>(*audio);
	}

	void OnActivated(const CoreApplicationView&, const IActivatedEventArgs&) {
		auto window = CoreWindow::GetForCurrentThread();
		window.Activate();
		renderer->setWindow(window);
	}

	void OnEnteredBackground(const IInspectable&, const EnteredBackgroundEventArgs&) {
		foreground = false;
	}

	void OnLeavingBackground(const IInspectable&, const LeavingBackgroundEventArgs&) {
		foreground = true;
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
			if (foreground) {
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
				game->update(duration_cast<milliseconds>(frameTime));
				renderer->clear();
				game->render(*renderer);
				renderer->present();
			} else {
				dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
			}
		}
	}

	void SetWindow(const CoreWindow& window) {
		window.SizeChanged({ this, &App::OnWindowSizeChanged });
		window.KeyDown({ this, &App::OnKeyDown });
		window.KeyUp({ this, &App::OnKeyUp });
		DisplayInformation::DisplayContentsInvalidated({ this, &App::OnDisplayContentsInvalidated });
		auto displayInfo = DisplayInformation::GetForCurrentView();
		displayInfo.DpiChanged({ this, &App::OnDPIChanged });
		renderer->setWindow(window);
	}

	void OnWindowSizeChanged(const CoreWindow& window, const WindowSizeChangedEventArgs&) {
		renderer->setWindowSize(Size(window.Bounds().Width, window.Bounds().Height));
	}

	void OnDisplayContentsInvalidated(const DisplayInformation&, const IInspectable&) {
		renderer->initDeviceResources();
		renderer->initWindowResources();
	}

	void OnDPIChanged(const DisplayInformation& info, const IInspectable&) {
		renderer->setDpi(info.LogicalDpi());
	}

	void OnKeyDown(const CoreWindow&, const KeyEventArgs& args) {
		game->onKeyDown(args);
	}

	void OnKeyUp(const CoreWindow&, const KeyEventArgs& args) {
		game->onKeyUp(args);
	}

	void OnGamepadAdded(const IInspectable&, const Gamepad& gamepad) {
		static const auto MaxPlayers = 2;
		critical_section::scoped_lock lock{ gamepadLock };
		if (gamepads.size() < MaxPlayers) {
			auto finder = std::find(begin(gamepads), end(gamepads), gamepad);
			if (finder == end(gamepads)) {
				gamepads.push_back(gamepad);
			}
		}
	}

	void OnGamepadRemoved(const IInspectable&, const Gamepad& gamepad) {
		critical_section::scoped_lock lock{ gamepadLock };
		auto finder = std::find(begin(gamepads), end(gamepads), gamepad);
		if (finder != end(gamepads)) {
			gamepads.erase(finder);
		}
	}

	void ReadGamepads() {
		critical_section::scoped_lock lock{ gamepadLock };
		for (auto i = 0; i < gamepads.size(); i++) {
			auto reading = gamepads[i].GetCurrentReading();
			game->onReadGamepad(i, reading);
		}
	}

private:
	std::unique_ptr<Renderer> renderer;
	std::unique_ptr<Audio>    audio;
	bool                      foreground = false;
	std::unique_ptr<Game>     game;
	critical_section          gamepadLock;
	std::vector<Gamepad>      gamepads;
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
	CoreApplication::Run(make<App>());
}
