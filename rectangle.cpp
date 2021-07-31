#include "pch.h"
#include "rectangle.h"

void Rectangle::render(float alpha, const Renderer::Ptr& renderer) const {
	auto ctx = renderer->getD2DContext();
	auto windowSize = renderer->getWindowSize();
	auto windowOffset = renderer->getWindowOffset();
	auto halfWidth = mWidth / 2;
	auto halfHeight = mHeight / 2;

	auto x = 0.f;
	auto y = 0.f;
	if (mStatic) {
		x = mX;
		y = mY;
	} else {
		x = mPreviousX + alpha * (mX - mPreviousX);
		y = mPreviousY + alpha * (mY - mPreviousY);
	}
	
	ctx->FillRectangle({
		windowOffset.Width + (-halfWidth + x) * (windowSize.Width - windowOffset.Width * 2),
		windowOffset.Height + (-halfHeight + y) * (windowSize.Height - windowOffset.Height * 2),
		windowOffset.Width + (halfWidth + x) * (windowSize.Width - windowOffset.Width * 2),
		windowOffset.Height + (halfHeight + y) * (windowSize.Height - windowOffset.Height * 2),
	}, mBrush.get());
}