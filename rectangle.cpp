#include "pch.h"
#include "rectangle.h"

void Rectangle::render(const Renderer::Ptr& renderer) const {
	auto ctx = renderer->getD2DContext();
	auto halfWidth = mWidth / 2;
	auto halfHeight = mHeight / 2;
	ctx->FillRectangle({ -halfWidth + mX, -halfHeight + mY, halfWidth + mX, halfHeight + mY}, mBrush.get());
}