#pragma once

#include "renderable.h"

class Text : public Renderable {
public:
	void render(float alpha, const Renderer::Ptr& renderer) const final;
	void setBrush(winrt::com_ptr<ID2D1Brush> brush) { mBrush = brush; }
	void setX(float x) { mX = x; }
	void setY(float y) { mY = y; }
	void setText(const std::wstring& text) { mText = text; }
private:
	winrt::com_ptr<ID2D1Brush>		  mBrush;
	float							  mX;
	float							  mY;
	std::wstring					  mText;
};