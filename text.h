#pragma once

#include "vec2f.h"

#include <d2d1.h>

struct Text {
	Vec2f					   position;
	std::wstring			   text;
	winrt::com_ptr<ID2D1Brush> brush;
	float					   fontSize;
};