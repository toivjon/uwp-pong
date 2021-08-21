#pragma once

#include "renderable.h"
#include "vec2f.h"

class Rectangle : public Renderable {
public:
	void render(float alpha, const Renderer::Ptr& renderer) const final;
	void setSize(const Vec2f& size) { mSize = size; }
	void setPosition(const Vec2f& position) { mPreviousPosition = position; mPosition = position; }
	void setStatic(bool enabled) { mStatic = enabled; }

	float getX() const { return mPosition.getX(); }
	float getY() const { return mPosition.getY(); }

	float getPreviousX() const { return mPreviousPosition.getX(); }
	float getPreviousY() const { return mPreviousPosition.getY(); }

	float getHalfWidth() const { return mSize.getX() / 2.f; }
	float getHalfHeight() const { return mSize.getY() / 2.f; }
private:
	Vec2f mSize;
	Vec2f mPosition;
	Vec2f mPreviousPosition;
	bool  mStatic;
};
