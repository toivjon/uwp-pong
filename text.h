#pragma once

#include "renderable.h"

class Text : public Renderable {
public:
	void render(const Renderer::Ptr& renderer) const final;
	void setBrush(winrt::com_ptr<ID2D1Brush> brush) { mBrush = brush; }
	void setFormat(winrt::com_ptr<IDWriteTextFormat> format) { mFormat = format;  }
	void setX(float x) { mX = x; }
	void setY(float y) { mY = y; }
	void setText(const std::wstring& text) { mText = text; }
private:
	winrt::com_ptr<ID2D1Brush>		  mBrush;
	winrt::com_ptr<IDWriteTextFormat> mFormat;
	float							  mX;
	float							  mY;
	std::wstring					  mText;
};