#pragma once

#include "renderable.h"
#include "vec2f.h"

class Rectangle : public Renderable {
public:
	void render(const Renderer::Ptr& renderer) const final;
	void setSize(const Vec2f& size) { mSize = size; }
	void setPosition(const Vec2f& position) { mPosition = position; }
	void setVelocity(const Vec2f& velocity) { mVelocity = velocity; }

	const Vec2f& getSize() const { return mSize; }
	const Vec2f& getPosition() const { return mPosition; }
	const Vec2f& getVelocity() const { return mVelocity; }
private:
	Vec2f mSize;
	Vec2f mPosition;
	Vec2f mVelocity;
};
