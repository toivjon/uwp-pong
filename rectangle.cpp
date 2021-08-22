#include "pch.h"
#include "rectangle.h"

void Rectangle::render(const Renderer::Ptr& renderer) const {
	auto ctx = renderer->getD2DContext();
	auto windowSize = renderer->getWindowSize();
	auto windowOffset = renderer->getWindowOffset();
	auto halfWidth = mSize.getX() / 2;
	auto halfHeight = mSize.getY() / 2;
	ctx->FillRectangle({
		windowOffset.Width + (-halfWidth + mPosition.getX()) * (windowSize.Width - windowOffset.Width * 2),
		windowOffset.Height + (-halfHeight + mPosition.getY()) * (windowSize.Height - windowOffset.Height * 2),
		windowOffset.Width + (halfWidth + mPosition.getX()) * (windowSize.Width - windowOffset.Width * 2),
		windowOffset.Height + (halfHeight + mPosition.getY()) * (windowSize.Height - windowOffset.Height * 2),
	}, renderer->getBrush().get());
}