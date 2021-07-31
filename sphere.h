#pragma once

#include "renderable.h"

class Sphere : public Renderable {
public:
	void render(float alpha, const Renderer::Ptr& renderer) const final;
	void setBrush(winrt::com_ptr<ID2D1Brush> brush) { mBrush = brush; }
	void setRadius(float radius) { mRadius = radius; }
	void setX(float x) { mPreviousX = mX; mX = x; }
	void setY(float y) { mPreviousY = mY; mY = y; }
	float getX() const { return mX; }
	float getY() const { return mY; }
private:
	winrt::com_ptr<ID2D1Brush> mBrush;
	float					   mRadius;
	float					   mPreviousX;
	float					   mPreviousY;
	float					   mX;
	float					   mY;
};
