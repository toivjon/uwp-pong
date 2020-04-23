#include "graphics.h"
#include "util.h"

#include <cassert>

using namespace D2D1;
using namespace Microsoft::WRL;
using namespace pong;
using namespace pong::graphics;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;

constexpr auto FONT_FAMILY = L"Calibri";
constexpr auto ASPECT_RATIO = (800.f / 600.f);

// TODO move to utils?
inline void ThrowIfFailed(HRESULT hr) {
	if (FAILED(hr)) {
		throw Platform::Exception::CreateException(hr);
	}
}

Graphics::Graphics() {
	InitDirect3D();
	InitDirect2D();
	InitDirectWrite();
	InitBrushes();
}

void Graphics::InitDirect3D() {
	assert(mD3DDevice == nullptr);
	assert(mD3DDeviceCtx == nullptr);

	// specify the desired additional behavior how the device will be created.
	UINT flags = 0;
	flags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT; // for Direct2D compatibility
	flags |= D3D11_CREATE_DEVICE_SINGLETHREADED;
#ifdef _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// specify the feature levels we want to support (ordering matters!).
	D3D_FEATURE_LEVEL featureLevels[] = {
	  D3D_FEATURE_LEVEL_11_1,
	  D3D_FEATURE_LEVEL_11_0,
	  D3D_FEATURE_LEVEL_10_1,
	  D3D_FEATURE_LEVEL_10_0,
	  D3D_FEATURE_LEVEL_9_3,
	  D3D_FEATURE_LEVEL_9_2,
	  D3D_FEATURE_LEVEL_9_1
	};

	// create a DirectX 11 device item.
	ThrowIfFailed(D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		flags,
		featureLevels,
		ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION,
		&mD3DDevice,
		nullptr,
		&mD3DDeviceCtx));
}

void Graphics::InitDirect2D() {
	assert(mD3DDevice != nullptr);
	assert(mD3DDeviceCtx != nullptr);
	assert(mD2DDevice == nullptr);
	assert(mD2DDeviceCtx == nullptr);
	assert(mD2DFactory == nullptr);

	// specify creation configuration for a new Direct2D factory.
	D2D1_FACTORY_OPTIONS options;
#ifdef _DEBUG
	options.debugLevel = D2D1_DEBUG_LEVEL_WARNING;
#endif

	// construct a new Direct2D factory to build Direct2D resources.
	ThrowIfFailed(D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		options,
		mD2DFactory.GetAddressOf()
	));

	// query the underlying DXGI device from the Direct3D device.
	ComPtr<IDXGIDevice3> dxgiDevice;
	ThrowIfFailed(mD3DDevice.As(&dxgiDevice));

	// create a Direct2D device and device context items.
	ThrowIfFailed(mD2DFactory->CreateDevice(dxgiDevice.Get(), mD2DDevice.GetAddressOf()));
	ThrowIfFailed(mD2DDevice->CreateDeviceContext(
		D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
		&mD2DDeviceCtx
	));
}

void Graphics::InitDirectWrite() {
	assert(mDWritefactory == nullptr);
	ThrowIfFailed(DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(mDWritefactory.GetAddressOf())
	));
}

void Graphics::InitBrushes() {
	assert(mD2DDeviceCtx != nullptr);
	ThrowIfFailed(mD2DDeviceCtx->CreateSolidColorBrush(ColorF(ColorF::White), &mWhiteBrush));
	ThrowIfFailed(mD2DDeviceCtx->CreateSolidColorBrush(ColorF(ColorF::Black), &mBlackBrush));
}

void Graphics::SetCoreWindow(CoreWindow^ window) {
	assert(window != nullptr);
	auto dpi = DisplayInformation::GetForCurrentView()->LogicalDpi;
	auto width = util::ConvertDipsToPixels(window->Bounds.Width, dpi);
	auto height = util::ConvertDipsToPixels(window->Bounds.Height, dpi);

	mD2DDeviceCtx->SetTarget(nullptr);

	if (mSwapChain) {
		mSwapChain->ResizeBuffers(
			2,
			static_cast<UINT>(width),
			static_cast<UINT>(height),
			DXGI_FORMAT_B8G8R8A8_UNORM,
			0
		);
	} else {
		// query the DXGI factory from our DirectX 11 device.
		ComPtr<IDXGIDevice1> dxgiDevice;
		ThrowIfFailed(mD3DDevice.As(&dxgiDevice));
		ComPtr<IDXGIAdapter> dxgiAdapter;
		ThrowIfFailed(dxgiDevice->GetAdapter(&dxgiAdapter));
		ComPtr<IDXGIFactory2> dxgiFactory;
		dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), &dxgiFactory);

		// specify swap chain configuration.
		DXGI_SWAP_CHAIN_DESC1 desc = {};
		desc.Width = static_cast<UINT>(width);
		desc.Height = static_cast<UINT>(height);
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = 2;
		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		desc.SampleDesc.Count = 1;

		// create the swap chain.
		ThrowIfFailed(dxgiFactory->CreateSwapChainForCoreWindow(
			mD3DDevice.Get(),
			reinterpret_cast<IUnknown*>(window),
			&desc,
			nullptr,
			&mSwapChain
		));
	}

	// construct a bitmap descriptor that is used with Direct2D rendering.
	D2D1_BITMAP_PROPERTIES1 properties = {};
	properties.bitmapOptions |= D2D1_BITMAP_OPTIONS_TARGET;
	properties.bitmapOptions |= D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
	properties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
	properties.dpiX = dpi;
	properties.dpiY = dpi;

	// query the DXGI version of the back buffer surface.
	ComPtr<IDXGISurface> dxgiBackBuffer;
	ThrowIfFailed(mSwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer)));

	// create a new bitmap that's going to be used by the Direct2D.
	ComPtr<ID2D1Bitmap1> bitmap;
	ThrowIfFailed(mD2DDeviceCtx->CreateBitmapFromDxgiSurface(
		dxgiBackBuffer.Get(),
		&properties,
		&bitmap
	));

	// assign the created bitmap as Direct2D render target.
	mD2DDeviceCtx->SetTarget(bitmap.Get());
	mD2DDeviceCtx->SetDpi(dpi, dpi);

	// FIXME duplicate code

	// calculate aspect ratio correction spacing values.
	auto currentAspectRatio = (width / height);
	auto widthSpacing = 0.f, heightSpacing = 0.f;
	if (currentAspectRatio > ASPECT_RATIO) {
		auto widthEdge = height * ASPECT_RATIO;
		widthSpacing = width - widthEdge;
	} else if (currentAspectRatio < ASPECT_RATIO) {
		auto heightEdge = width / ASPECT_RATIO;
		heightSpacing = height - heightEdge;
	}

	// (re)create text formats.
	auto cellSize = (height - heightSpacing) / 30;
	BuildTextFormat(cellSize * 6.00f, mBigTextFormat);
	BuildTextFormat(cellSize * 3.00f, mMediumTextFormat);
	BuildTextFormat(cellSize * 0.75f, mSmallTextFormat);
}

void Graphics::BeginDrawAndClear() {
	mD2DDeviceCtx->BeginDraw();
	mD2DDeviceCtx->Clear(D2D1::ColorF(D2D1::ColorF::Black));
}

void Graphics::EndDrawAndPresent() {
	ThrowIfFailed(mD2DDeviceCtx->EndDraw());
	ThrowIfFailed(mSwapChain->Present(1, 0));
}

void Graphics::DrawWhiteRects(const std::vector<geometry::Rectangle> rects) {
	for (auto& rect : rects) {
		FillWhiteRect(rect);
	}
}

void Graphics::FillWhiteRect(const D2D1_RECT_F& rect) {
	mD2DDeviceCtx->FillRectangle(rect, mWhiteBrush.Get());
}

void Graphics::DrawWhiteBigText(const std::wstring& text, const D2D1_RECT_F& rect) {
	assert(mD2DDeviceCtx != nullptr);
	assert(mBigTextFormat != nullptr);
	assert(mWhiteBrush != nullptr);
	mD2DDeviceCtx->DrawText(
		text.c_str(),
		static_cast<UINT32>(text.size()),
		mBigTextFormat.Get(),
		rect,
		mWhiteBrush.Get()
	);
}

void Graphics::DrawBlackSmallText(const std::wstring& text, const D2D1_RECT_F& rect) {
	assert(mD2DDeviceCtx != nullptr);
	assert(mSmallTextFormat != nullptr);
	assert(mBlackBrush != nullptr);
	mD2DDeviceCtx->DrawText(
		text.c_str(),
		static_cast<UINT32>(text.size()),
		mSmallTextFormat.Get(),
		rect,
		mBlackBrush.Get()
	);
}

void Graphics::DrawBlackMediumText(const std::wstring& text, const D2D1_RECT_F& rect) {
	assert(mD2DDeviceCtx != nullptr);
	assert(mMediumTextFormat != nullptr);
	assert(mBlackBrush != nullptr);
	mD2DDeviceCtx->DrawText(
		text.c_str(),
		static_cast<UINT32>(text.size()),
		mMediumTextFormat.Get(),
		rect,
		mBlackBrush.Get()
	);
}

void Graphics::BuildTextFormat(float size, ComPtr<IDWriteTextFormat>& format) {
	assert(mDWritefactory != nullptr);
	ThrowIfFailed(mDWritefactory->CreateTextFormat(
		FONT_FAMILY,
		nullptr,
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		size,
		L"en-us",
		&format
	));
	ThrowIfFailed(format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
}
