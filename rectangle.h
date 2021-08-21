#pragma once

#include "renderable.h"
#include "vec2f.h"

class Rectangle : public Renderable {
public:
	void render(float alpha, const Renderer::Ptr& renderer) const final;
	void setSize(const Vec2f& size) { mSize = size; }
	void setX(float x) { mPreviousX = mX;  mX = x; }
	void setY(float y) { mPreviousY = mY;  mY = y; }
	void setStatic(bool enabled) { mStatic = enabled; }

	float getX() const { return mX; }
	float getY() const { return mY; }

	float getPreviousX() const { return mPreviousX; }
	float getPreviousY() const { return mPreviousY; }

	float getHalfWidth() const { return mSize.getX() / 2.f; }
	float getHalfHeight() const { return mSize.getY() / 2.f; }
private:
	Vec2f mSize;
	float mPreviousX;
	float mPreviousY;
	float mX;
	float mY;
	bool  mStatic;
};
