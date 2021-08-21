#pragma once

#include "renderable.h"

class Rectangle : public Renderable {
public:
	void render(float alpha, const Renderer::Ptr& renderer) const final;
	void setWidth(float width) { mWidth = width; }
	void setHeight(float height) { mHeight = height; }
	void setX(float x) { mPreviousX = mX;  mX = x; }
	void setY(float y) { mPreviousY = mY;  mY = y; }
	void setStatic(bool enabled) { mStatic = enabled; }

	float getX() const { return mX; }
	float getY() const { return mY; }

	float getPreviousX() const { return mPreviousX; }
	float getPreviousY() const { return mPreviousY; }

	float getHalfWidth() const { return mWidth / 2.f; }
	float getHalfHeight() const { return mHeight / 2.f; }
private:
	float					   mWidth;
	float					   mHeight;
	float					   mPreviousX;
	float					   mPreviousY;
	float					   mX;
	float					   mY;
	bool					   mStatic;
};
