#pragma once

#include "renderable.h"
#include "vec2f.h"

class Text : public Renderable {
public:
	void render(const Renderer::Ptr& renderer) const final;
	void setPosition(const Vec2f& position) { mPosition = position; }
	void setText(const std::wstring& text) { mText = text; }
	void setBrush(winrt::com_ptr<ID2D1Brush> brush) { mBrush = brush; }
	void setFontSize(float fontSize) { mFontSize = fontSize; }
private:
	Vec2f		 mPosition;
	std::wstring mText;
	winrt::com_ptr<ID2D1Brush> mBrush;
	float mFontSize;
};