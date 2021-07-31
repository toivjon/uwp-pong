#pragma once

#include "renderer.h"

// An interface for all renderable types. Requires an implementation of the render function.
class Renderable {
public:
	virtual void render(float alpha, const Renderer::Ptr& renderer) const = 0;
};