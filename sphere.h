#pragma once

#include "renderable.h"

class Sphere : public Renderable {
public:
	void render(const Renderer::Ptr& renderer) const final;
	void setBrush(winrt::com_ptr<ID2D1Brush> brush) { mBrush = brush; }
	void setRadius(float radius) { mRadius = radius; }
private:
	winrt::com_ptr<ID2D1Brush> mBrush;
	float					   mRadius;
};
