#pragma once

#include <string>
#include <cstdint>

namespace Pong
{
	class Player final
	{
	public:
		Player();

		void ResetName();

		void SetScore(uint8_t score) { mScore = score; }
		uint8_t GetScore() const	 { return mScore;  }
		void IncrementScore()		 { mScore++;	   }

		void SetName(const std::wstring& name)	{ mName = name; }
		const std::wstring& GetName() const		{ return mName; }
	private:
		uint8_t		 mScore;
		std::wstring mName;
	};
}
