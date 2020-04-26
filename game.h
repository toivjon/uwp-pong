#pragma once

namespace Pong
{
	class Game final
	{
	public:
		void Run();
		void Stop();
		void Pause();
		void Resume();
		void SetResolution(float width, float height);
	};
}
