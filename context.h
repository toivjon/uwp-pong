#pragma once

#include <string>

namespace pong
{
	class Context final
	{
	public:
		void Update(unsigned long dt);

		void Render(double alpha);

		void ChangeScene(const std::string& name);

		void PlaySound(const std::string& name);
	private:
	};
}
