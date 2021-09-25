#pragma once

#include "renderer.h"
#include "vec2f.h"

struct Text {
	Vec2f					   position;
	std::wstring			   text;
	winrt::com_ptr<ID2D1Brush> brush;
	float					   fontSize;

	void render(const Renderer::Ptr& renderer) const;
};