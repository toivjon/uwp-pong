#include "game.h"

using namespace pong;

using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;

IFrameworkView^ Game::CreateView() {
	return ref new Game();
}

void Game::Initialize(Windows::ApplicationModel::Core::CoreApplicationView^)
{
	// ...
}

void Game::SetWindow(CoreWindow^ window)
{
	window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &Game::OnWindowClosed);
}

void Game::Load(Platform::String^)
{
	// ...
}

void Game::Run()
{
	while (!mWindowClosed) {
		auto window = CoreWindow::GetForCurrentThread();
		window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
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