#pragma once

#include <d2d1_1.h>

namespace pong::geometry
{
	struct Rectangle : public D2D1_RECT_F
	{
		bool Contains(float x, float y) const;
		bool Contains(const Rectangle& rect) const;
		bool Collides(const Rectangle& rect) const;
		bool Collides(const D2D_RECT_F& rect) const;
		void Set(float left, float top, float right, float bottom);
		void Move(float x, float y);

		static Rectangle Lerp(const Rectangle& r1, const Rectangle& r2, float t);
	};
}