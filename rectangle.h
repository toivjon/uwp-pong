#pragma once

#include "renderable.h"
#include "vec2f.h"

class Rectangle : public Renderable {
public:
	void render(float alpha, const Renderer::Ptr& renderer) const final;
	void setSize(const Vec2f& size) { mSize = size; }
	void setPosition(const Vec2f& position) { mPreviousPosition = position; mPosition = position; }
	void setStatic(bool enabled) { mStatic = enabled; }

	const Vec2f& getSize() const { return mSize; }
	const Vec2f& getPosition() const { return mPosition; }
	const Vec2f& getPreviousPosition() const { return mPreviousPosition; }
private:
	Vec2f mSize;
	Vec2f mPosition;
	Vec2f mPreviousPosition;
	bool  mStatic;
};
