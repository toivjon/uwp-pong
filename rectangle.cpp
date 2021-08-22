#include "pch.h"
#include "rectangle.h"

void Rectangle::render(float alpha, const Renderer::Ptr& renderer) const {
	auto ctx = renderer->getD2DContext();
	auto windowSize = renderer->getWindowSize();
	auto windowOffset = renderer->getWindowOffset();
	auto halfWidth = mSize.getX() / 2;
	auto halfHeight = mSize.getY() / 2;
	auto position = Vec2f::lerp(mPreviousPosition, mPosition, alpha);
	ctx->FillRectangle({
		windowOffset.Width + (-halfWidth + position.getX()) * (windowSize.Width - windowOffset.Width * 2),
		windowOffset.Height + (-halfHeight + position.getY()) * (windowSize.Height - windowOffset.Height * 2),
		windowOffset.Width + (halfWidth + position.getX()) * (windowSize.Width - windowOffset.Width * 2),
		windowOffset.Height + (halfHeight + position.getY()) * (windowSize.Height - windowOffset.Height * 2),
	}, renderer->getBrush().get());
}