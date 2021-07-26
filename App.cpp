#include "pch.h"
#include "renderer.h"
#include "sphere.h"
#include "rectangle.h"
#include "scene.h"
#include "text.h"

// The update interval frequency in seconds.
const long long UpdateIntervalSeconds = 10;

// The maximum allowed time for one frame update.
const long long MaxFrameTime = 250;

using namespace winrt;

using namespace Windows;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::UI;
using namespace Windows::UI::Core;

long long CurrentMillis() {
	using namespace std::chrono;
	auto time = system_clock::now().time_since_epoch();
	auto ms = duration_cast<milliseconds>(time);
	return ms.count();
}

struct App : implements<App, IFrameworkViewSource, IFrameworkView> {

	IFrameworkView CreateView() {
		return *this;
	}

	void Initialize(const CoreApplicationView& view) {
		OutputDebugStringA("App::Initialize\n");
		view.Activated({ this, &App::OnActivated });
		CoreApplication::EnteredBackground({ this, &App::OnEnteredBackground });
		CoreApplication::LeavingBackground({ this, &App::OnLeavingBackground });
		CoreApplication::Suspending({ this, &App::OnSuspending });
		CoreApplication::Resuming({ this, &App::OnResuming });
		mRenderer = std::make_unique<Renderer>();
		mScene = std::make_unique<Scene>(mRenderer);
	}

	void OnActivated(const CoreApplicationView&, const IActivatedEventArgs&) {
		OutputDebugStringA("App::OnActivated\n");
		CoreWindow::GetForCurrentThread().Activate();
	}

	void OnEnteredBackground(const IInspectable&, const EnteredBackgroundEventArgs&) {
		OutputDebugStringA("App::OnEnteredBackground\n");
		mForeground = false;
		// TODO pause simulation, store state and release resources.
	}

	void OnLeavingBackground(const IInspectable&, const LeavingBackgroundEventArgs&) {
		OutputDebugStringA("App::OnLeavingBackground\n");
		mForeground = true;
		// TODO resume simulation, restore state and acquire resources.
	}

	void OnSuspending(const IInspectable&, const SuspendingEventArgs&) {
		OutputDebugStringA("App::OnSuspend\n");
		// TODO pause simulation, store state and release resources.
	}

	void OnResuming(const IInspectable&, const IInspectable&) {
		OutputDebugStringA("App::OnResuming\n");
		// TODO resume simulation, restore state and acquire resources.
	}

	void Load(const hstring&) {
		OutputDebugStringA("App::Load\n");
	}

	void Uninitialize() {
		// TODO
	}

	void Run() {
		auto prevTime = CurrentMillis();
		auto accumulator = 0ll;
		while (true) {
			auto window = CoreWindow::GetForCurrentThread();
			auto dispatcher = window.Dispatcher();
			if (mForeground) {
				dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

				// Update time definitions and delta calculations.
				auto time = CurrentMillis();
				auto dt = time - prevTime;
				if (dt > MaxFrameTime) {
					dt = MaxFrameTime;
				}
				prevTime = time;
				accumulator += dt;

				// Update game logic with fixed timesteps.
				while (accumulator >= UpdateIntervalSeconds) {
					mScene->update(UpdateIntervalSeconds);
					accumulator -= UpdateIntervalSeconds;
				}

				// Render scene contents.
				mRenderer->clear();
				mScene->render(mRenderer);
				mRenderer->present();
			} else {
				dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
			}
		}
	}

	void SetWindow(const CoreWindow& window) {
		OutputDebugStringA("App::SetWindow\n");
		window.SizeChanged({ this, &App::OnWindowSizeChanged });
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

private:
	Renderer::Ptr mRenderer;
	bool		  mForeground = false;
	Scene::Ptr	  mScene;
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
	CoreApplication::Run(make<App>());
}
