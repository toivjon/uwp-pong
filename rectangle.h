#pragma once

#include "renderable.h"

class Rectangle : public Renderable {
public:
	void render(float alpha, const Renderer::Ptr& renderer) const final;
	void setBrush(winrt::com_ptr<ID2D1Brush> brush) { mBrush = brush; }
	void setWidth(float width) { mWidth = width; }
	void setHeight(float height) { mHeight = height; }
	void setX(float x) { mX = x; }
	void setY(float y) { mY = y; }
private:
	winrt::com_ptr<ID2D1Brush> mBrush;
	float					   mWidth;
	float					   mHeight;
	float					   mX;
	float					   mY;
};
