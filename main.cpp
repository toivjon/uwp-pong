#include "game.h"

using namespace pong;
using namespace Windows::ApplicationModel::Core;

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	CoreApplication::Run(ref new Game());
	return 0;
}
