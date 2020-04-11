#pragma once

#include <d2d1_1.h>

namespace pong::geometry
{
	class Rectangle final : public D2D1_RECT_F
	{
	public:
		bool Contains(float x, float y) const;
		bool Contains(const Rectangle& rect) const;
		bool Collides(const Rectangle& rect) const;
		void Set(float left, float top, float right, float bottom);
		void Move(float x, float y);
	};
}