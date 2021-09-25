#include "pch.h"
#include "rectangle.h"

void Rectangle::render(const Renderer::Ptr& renderer) const {
	auto ctx = renderer->getD2DContext();
	auto windowSize = renderer->getWindowSize();
	auto windowOffset = renderer->getWindowOffset();
	auto halfWidth = size.getX() / 2;
	auto halfHeight = size.getY() / 2;
	ctx->FillRectangle({
		windowOffset.Width + (-halfWidth + position.getX()) * (windowSize.Width - windowOffset.Width * 2),
		windowOffset.Height + (-halfHeight + position.getY()) * (windowSize.Height - windowOffset.Height * 2),
		windowOffset.Width + (halfWidth + position.getX()) * (windowSize.Width - windowOffset.Width * 2),
		windowOffset.Height + (halfHeight + position.getY()) * (windowSize.Height - windowOffset.Height * 2),
	}, brush.get());
}