#include "pch.h"
#include "rectangle.h"

void Rectangle::render(float alpha, const Renderer::Ptr& renderer) const {
	auto ctx = renderer->getD2DContext();
	auto windowSize = renderer->getWindowSize();
	auto windowOffset = renderer->getWindowOffset();
	auto halfWidth = mWidth / 2;
	auto halfHeight = mHeight / 2;
	ctx->FillRectangle({
		windowOffset.Width + (-halfWidth + mX) * (windowSize.Width - windowOffset.Width * 2),
		windowOffset.Height + (-halfHeight + mY) * (windowSize.Height - windowOffset.Height * 2),
		windowOffset.Width + (halfWidth + mX) * (windowSize.Width - windowOffset.Width * 2),
		windowOffset.Height + (halfHeight + mY) * (windowSize.Height - windowOffset.Height * 2),
	}, mBrush.get());
}