#include "pch.h"
#include "sphere.h"

void Sphere::render(const Renderer::Ptr& renderer) const {
	auto ctx = renderer->getD2DContext();
	ctx->FillEllipse({ {0,0}, mRadius, mRadius }, mBrush.get());
}