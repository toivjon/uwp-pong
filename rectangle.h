#pragma once

#include "renderable.h"

class Rectangle : public Renderable {
public:
	void render(const Renderer::Ptr& renderer) const final;
	void setBrush(winrt::com_ptr<ID2D1Brush> brush) { mBrush = brush; }
	void setWidth(float width) { mWidth = width; }
	void setHeight(float height) { mHeight = height; }
private:
	winrt::com_ptr<ID2D1Brush> mBrush;
	float					   mWidth;
	float					   mHeight;
};
