#include "game.h"

using namespace pong;

using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;

IFrameworkView^ Game::CreateView() {
	return ref new Game();
}

void Game::Initialize(CoreApplicationView^ view)
{
	view->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &Game::OnActivated);
	mAudio = std::make_unique<Audio>();
}

void Game::SetWindow(CoreWindow^ window)
{
	window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &Game::OnWindowClosed);
	window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &Game::OnWindowVisibilityChanged);
}

void Game::Load(Platform::String^)
{
	// ...
}

void Game::Run()
{
	while (!mWindowClosed) {
		auto window = CoreWindow::GetForCurrentThread();
		if (mWindowVisible) {
			window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
			// TODO update
			// TODO render
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