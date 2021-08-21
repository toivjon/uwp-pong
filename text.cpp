#include "pch.h"
#include "text.h"

void Text::render(float alpha, const Renderer::Ptr& renderer) const {
	auto ctx = renderer->getD2DContext();
	auto windowSize = renderer->getWindowSize();
	auto windowOffset = renderer->getWindowOffset();
	auto writeFactory = renderer->getDWriteFactory();
	auto size = 0.27f * (windowSize.Height - windowOffset.Height * 2.f);
	winrt::com_ptr<IDWriteTextFormat> format;
	writeFactory->CreateTextFormat(
		L"Calibri",
		nullptr,
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		size,
		L"en-us",
		format.put()
	);
	format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	auto x = windowOffset.Width + mX * (windowSize.Width - windowOffset.Width * 2);
	auto y = windowOffset.Height + mY * (windowSize.Height - windowOffset.Height * 2);
	ctx->DrawText(
		mText.c_str(),
		UINT32(mText.size()),
		format.get(),
		{ x,y,x,y },
		renderer->getBrush().get()
	);
}