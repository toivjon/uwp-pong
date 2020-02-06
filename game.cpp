#include "game.h"

using namespace pong;

using namespace Windows::ApplicationModel::Core;

IFrameworkView^ Game::CreateView() {
	return ref new Game();
}

void Game::Initialize(Windows::ApplicationModel::Core::CoreApplicationView^)
{
	// ...
}

void Game::SetWindow(Windows::UI::Core::CoreWindow^)
{
	// ...
}

void Game::Load(Platform::String^)
{
	// ...
}

void Game::Run()
{
	// ...
}

void Game::Uninitialize()
{
	// ...
}
