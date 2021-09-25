#pragma once

#include "vec2f.h"

#include <d2d1.h>

struct Rectangle {
public:
	Vec2f					   size;
	Vec2f					   position;
	Vec2f					   velocity;
	winrt::com_ptr<ID2D1Brush> brush;
};
