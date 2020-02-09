#pragma once

#include <string>

namespace pong
{
	class Context final
	{
	public:
		void Update();

		void Render();

		void ChangeScene(const std::string& name);

		void PlaySound(const std::string& name);
	private:

	};
}
