#pragma once

#include "renderer.h"

// An interface for all renderable types. Requires an implementation of the Render function.
class Renderable {
public:
	virtual void Render(const Renderer& renderer) const = 0;
};