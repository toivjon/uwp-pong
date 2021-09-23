#pragma once

#include "aabb.h"
#include "renderable.h"
#include "vec2f.h"

class Rectangle : public Renderable {
public:
	void render(const Renderer::Ptr& renderer) const final;
	void setSize(const Vec2f& size) { mSize = size; }
	void setPosition(const Vec2f& position) { mPosition = position; }
	void setVelocity(const Vec2f& velocity) { mVelocity = velocity; }
	void setPositionX(float x) { mPosition.setX(x); }
	void setPositionY(float y) { mPosition.setY(y); }
	void setVelocityX(float x) { mVelocity.setX(x); }
	void setVelocityY(float y) { mVelocity.setY(y); }
	void setBrush(winrt::com_ptr<ID2D1Brush> brush) { mBrush = brush; }

	const Vec2f& getSize() const { return mSize; }
	const Vec2f& getPosition() const { return mPosition; }
	const Vec2f& getVelocity() const { return mVelocity; }
	const Vec2f& getExtent() const { return mSize / 2.f; }
	const AABB& getAABB() const { return AABB(mPosition, getExtent()); }
private:
	Vec2f mSize;
	Vec2f mPosition;
	Vec2f mVelocity;
	winrt::com_ptr<ID2D1Brush> mBrush;
};
