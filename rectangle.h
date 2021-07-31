#pragma once

#include "renderable.h"

class Rectangle : public Renderable {
public:
	void render(float alpha, const Renderer::Ptr& renderer) const final;
	void setBrush(winrt::com_ptr<ID2D1Brush> brush) { mBrush = brush; }
	void setWidth(float width) { mWidth = width; }
	void setHeight(float height) { mHeight = height; }
	void setX(float x) { mPreviousX = mX;  mX = x; }
	void setY(float y) { mPreviousY = mY;  mY = y; }
	void setStatic(bool enabled) { mStatic = enabled; }

	float getX() const { return mX; }
	float getY() const { return mY; }
private:
	winrt::com_ptr<ID2D1Brush> mBrush;
	float					   mWidth;
	float					   mHeight;
	float					   mPreviousX;
	float					   mPreviousY;
	float					   mX;
	float					   mY;
	bool					   mStatic;
};
