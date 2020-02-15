#include "game.h"
#include "util.h"

using namespace pong;

using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::Gaming::Input;
using namespace Windows::UI::Core;

// The amount of update operations per second.
constexpr auto UPDATES_PER_SECOND = 25;

// The update physical step size in milliseconds.
constexpr auto UPDATE_MILLIS = 1000 / 25;

IFrameworkView^ Game::CreateView() {
	return ref new Game();
}

void Game::Initialize(CoreApplicationView^ view)
{
	view->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &Game::OnActivated);
	mAudio = std::make_unique<Audio>();
	mGraphics = std::make_unique<Graphics>();
	Gamepad::GamepadAdded += ref new EventHandler<Gamepad^>(this, &Game::OnGamepadAdded);
	Gamepad::GamepadRemoved += ref new EventHandler<Gamepad^>(this, &Game::OnGamepadRemoved);
}

void Game::SetWindow(CoreWindow^ window)
{
	window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &Game::OnWindowClosed);
	window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &Game::OnWindowVisibilityChanged);
	mGraphics->SetWindow(window);
}

void Game::Load(Platform::String^)
{
	// ...
}

void Game::Run()
{
	auto millisAccumulator = 0l;
	auto currentMillis = CurrentMillis();
	while (!mWindowClosed) {
		auto window = CoreWindow::GetForCurrentThread();
		if (mWindowVisible) {
			window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
			mContext.GetRightPlayer().CheckInput(mContext);
			mContext.GetLeftPlayer().CheckInput(mContext);

			// calculate the time usable for the current frame.
			auto now = CurrentMillis();
			auto delta = min(now - currentMillis, 100);
			currentMillis = now;
			millisAccumulator += delta;

			// perform ticking of the game logic and physics.
			while (millisAccumulator >= UPDATE_MILLIS) {
				mContext.Update(UPDATE_MILLIS);
				millisAccumulator -= UPDATE_MILLIS;
			}

			// perform interpolated rendering of the game scene.
			auto alpha = double(millisAccumulator) / double(UPDATE_MILLIS);
			mContext.Render(alpha);
			mGraphics->Present();
		} else {
			window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
		}
	}
}

void Game::Uninitialize()
{
	// ...
}

void Game::OnWindowClosed(CoreWindow^, CoreWindowEventArgs^)
{
	mWindowClosed = true;
}

void Game::OnActivated(CoreApplicationView^, IActivatedEventArgs^)
{
	CoreWindow::GetForCurrentThread()->Activate();
}

void Game::OnWindowVisibilityChanged(CoreWindow^, VisibilityChangedEventArgs^ args)
{
	mWindowVisible = args->Visible;
}

void Game::OnGamepadAdded(Object^ o, Gamepad^ gamepad)
{
	Event e;
	e.Type = EventType::GamepadAdded;
	e.Priority = 0;
	e.Args = GamepadEvent{ gamepad };
	mContext.EnqueueEvent(e);
}

void Game::OnGamepadRemoved(Object^ o, Gamepad^ gamepad)
{
	Event e;
	e.Type = EventType::GamepadRemoved;
	e.Priority = 0;
	e.Args = GamepadEvent{ gamepad };
	mContext.EnqueueEvent(e);
}