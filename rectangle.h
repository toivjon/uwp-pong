#pragma once

#include "aabb.h"
#include "renderer.h"
#include "vec2f.h"

struct Rectangle {
public:
	Vec2f					   size;
	Vec2f					   position;
	Vec2f					   velocity;
	winrt::com_ptr<ID2D1Brush> brush;

	void render(const Renderer::Ptr& renderer) const;
};
