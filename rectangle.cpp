#include "pch.h"
#include "rectangle.h"

void Rectangle::render(const Renderer::Ptr& renderer) const {
	auto ctx = renderer->getD2DContext();
	ctx->FillRectangle({ mWidth / 2, mHeight / 2 }, mBrush.get());
}