#pragma once

#include "renderable.h"

class Sphere : public Renderable {
public:
	void render(const Renderer::Ptr& renderer) const final;
	void setBrush(winrt::com_ptr<ID2D1Brush> brush) { mBrush = brush; }
	void setRadius(float radius) { mRadius = radius; }
	void setX(float x) { mX = x; }
	void setY(float y) { mY = y; }
	float getX() const { return mX; }
	float getY() const { return mY; }
private:
	winrt::com_ptr<ID2D1Brush> mBrush;
	float					   mRadius;
	float					   mX;
	float					   mY;
};
