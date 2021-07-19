#include "pch.h"
#include "sphere.h"

void Sphere::render(const Renderer::Ptr& renderer) const {
	auto ctx = renderer->getD2DContext();
	auto windowSize = renderer->getWindowSize();
	auto windowOffset = renderer->getWindowOffset();
	auto radius = mRadius * (windowSize.Width - windowOffset.Width * 2);
	ctx->FillEllipse({ {
			windowOffset.Width + mX * (windowSize.Width - windowOffset.Width * 2),
			windowOffset.Height + mY * (windowSize.Height - windowOffset.Height * 2),
		},
		radius,
		radius
	}, mBrush.get());
}