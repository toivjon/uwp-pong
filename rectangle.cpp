#include "pch.h"
#include "rectangle.h"

void Rectangle::render(float alpha, const Renderer::Ptr& renderer) const {
	auto ctx = renderer->getD2DContext();
	auto windowSize = renderer->getWindowSize();
	auto windowOffset = renderer->getWindowOffset();
	auto halfWidth = mSize.getX() / 2;
	auto halfHeight = mSize.getY() / 2;

	auto x = 0.f;
	auto y = 0.f;
	if (mStatic) {
		x = mPosition.getX();
		y = mPosition.getY();
	} else {
		x = mPreviousPosition.getX() + alpha * (mPosition.getX() - mPreviousPosition.getX());
		y = mPreviousPosition.getY() + alpha * (mPosition.getY() - mPreviousPosition.getY());
	}
	
	ctx->FillRectangle({
		windowOffset.Width + (-halfWidth + x) * (windowSize.Width - windowOffset.Width * 2),
		windowOffset.Height + (-halfHeight + y) * (windowSize.Height - windowOffset.Height * 2),
		windowOffset.Width + (halfWidth + x) * (windowSize.Width - windowOffset.Width * 2),
		windowOffset.Height + (halfHeight + y) * (windowSize.Height - windowOffset.Height * 2),
	}, renderer->getBrush().get());
}