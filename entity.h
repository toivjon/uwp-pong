#pragma once

#include <d2d1_1.h>

namespace pong
{
	class Entity final
	{
	public:
		void SetPosition(float x, float y);
		void SetSize(float w, float h);

		float GetY() const { return mRect.top;  }
		float GetX() const { return mRect.left; }

		float GetWidth()  const	{ return mRect.right - mRect.left; }
		float GetHeight() const { return mRect.bottom - mRect.top; }

		const D2D_RECT_F& GetRect() const { return mRect; }

		bool Collides(const D2D_RECT_F& rect) const;
		bool Contains(const D2D_RECT_F& rect) const;
		bool Contains(float x, float y) const;
	private:
		D2D_RECT_F mRect;
	};
}
