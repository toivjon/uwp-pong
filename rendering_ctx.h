#pragma once

namespace pong
{
	class RenderingCtx final
	{
	public:
		void SetAlpha(float alpha) { mAlpha = alpha; }

		float GetAlpha() const { return mAlpha; }
	private:
		float mAlpha;
	};
}
