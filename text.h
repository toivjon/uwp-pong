#pragma once

#include "renderable.h"
#include "vec2f.h"

class Text : public Renderable {
public:
	void render(float alpha, const Renderer::Ptr& renderer) const final;
	void setPosition(const Vec2f& position) { mPosition = position; }
	void setText(const std::wstring& text) { mText = text; }
private:
	Vec2f		 mPosition;
	std::wstring mText;
};