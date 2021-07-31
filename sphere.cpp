#include "pch.h"
#include "sphere.h"

void Sphere::render(float alpha, const Renderer::Ptr& renderer) const {
	auto ctx = renderer->getD2DContext();
	auto windowSize = renderer->getWindowSize();
	auto windowOffset = renderer->getWindowOffset();
	auto radius = mRadius * (windowSize.Width - windowOffset.Width * 2);

	auto x = mPreviousX + alpha * (mX - mPreviousX);
	auto y = mPreviousY + alpha * (mY - mPreviousY);

	ctx->FillEllipse({ {
			windowOffset.Width + x * (windowSize.Width - windowOffset.Width * 2),
			windowOffset.Height + y * (windowSize.Height - windowOffset.Height * 2),
		},
		radius,
		radius
	}, mBrush.get());
}