#include "pch.h"
#include "text.h"

void Text::render(const Renderer::Ptr& renderer) const {
	auto ctx = renderer->getD2DContext();
	auto windowSize = renderer->getWindowSize();
	auto windowOffset = renderer->getWindowOffset();
	auto x = windowOffset.Width + mX * (windowSize.Width - windowOffset.Width * 2);
	auto y = windowOffset.Height + mY * (windowSize.Height - windowOffset.Height * 2);
	ctx->DrawText(
		mText.c_str(),
		UINT32(mText.size()),
		mFormat.get(),
		{ x,y,x,y },
		mBrush.get()
	);
}