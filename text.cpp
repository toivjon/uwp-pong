#include "pch.h"
#include "text.h"

void Text::render(const Renderer::Ptr& renderer) const {
	auto ctx = renderer->getD2DContext();
	auto windowSize = renderer->getWindowSize();
	auto windowOffset = renderer->getWindowOffset();
	auto writeFactory = renderer->getDWriteFactory();
	auto size = fontSize * (windowSize.Height - windowOffset.Height * 2.f);
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
	auto x = windowOffset.Width + position.getX() * (windowSize.Width - windowOffset.Width * 2);
	auto y = windowOffset.Height + position.getY() * (windowSize.Height - windowOffset.Height * 2);
	ctx->DrawText(
		text.c_str(),
		UINT32(text.size()),
		format.get(),
		{ x-text.length() * size * .5f,y,x + text.length() * size * .5f,y},
		brush.get()
	);
}