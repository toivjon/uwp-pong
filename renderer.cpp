#include "pch.hpp"
#include "renderer.hpp"

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics::Display;

// An array of supported DirectX hardware feature levels. Order matters.
constexpr D3D_FEATURE_LEVEL FeatureLevels[] = {
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_10_0,
	D3D_FEATURE_LEVEL_9_3,
	D3D_FEATURE_LEVEL_9_2,
	D3D_FEATURE_LEVEL_9_1
};

Renderer::Renderer() {
	OutputDebugStringA("GraphicsContext::GraphicsContext\n");

	// Specify the desired additional behavior how the Direct2D factory will be created.
	D2D1_FACTORY_OPTIONS options{};
	#if defined(_DEBUG)
	options.debugLevel = D2D1_DEBUG_LEVEL_WARNING;
	#endif

	// Construct a new Direct2D factory to build Direct2D resources.
	check_hresult(D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		options,
		m2DFactory.put()
	));

	// Construct a new DirectDraw factory to build DirectDraw resources.
	check_hresult(DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory3),
		reinterpret_cast<::IUnknown**>(mDWriteFactory.put())
	));

	initDeviceResources();

	// Build the white and black brush, which are the only brushes we need.
	check_hresult(m2DDeviceCtx->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), mWhiteBrush.put()));
	check_hresult(m2DDeviceCtx->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), mBlackBrush.put()));
}

void Renderer::initDeviceResources() {
	OutputDebugStringA("GraphicsContext::initDeviceResources\n");

	// clear all possible old definitions.
	mSwapChain = nullptr;
	m2DDeviceCtx = nullptr;
	m2DDevice = nullptr;
	m3DDeviceCtx = nullptr;
	m3DDevice = nullptr;

	// Specify the desired additional behaviour how the device will be created.
	UINT flags = 0;
	flags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT; // for Direct2D compatibility
	#if defined(_DEBUG)
	flags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif

	// Construct a new Direct3D device and device context.
	check_hresult(D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		0,
		flags,
		FeatureLevels,
		ARRAYSIZE(FeatureLevels),
		D3D11_SDK_VERSION,
		m3DDevice.put(),
		nullptr,
		m3DDeviceCtx.put()
	));

	// Query and use the underlying DXGI device to create a Direct2D device.
	com_ptr<IDXGIDevice3> dxgiDevice;
	check_bool(m3DDevice.try_as(dxgiDevice));
	check_hresult(m2DFactory->CreateDevice(dxgiDevice.get(), m2DDevice.put()));

	// Construct a new Direct2D device context.
	check_hresult(m2DDevice->CreateDeviceContext(
		D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
		m2DDeviceCtx.put()
	));
}

void Renderer::initWindowResources() {
	OutputDebugStringA("GraphicsContext::initWindowResources\n");
	m2DDeviceCtx->SetTarget(nullptr);
	m3DDeviceCtx->Flush();

	// TODO perhaps we could adjust this in some other way?
	// calculate window entity offset to maintain aspect ratio.
	mWindowOffset = { 0,0 };
	auto aspect = mWindowSize.Width / mWindowSize.Height;
	const auto desiredAspect = 1.3f;
	if (abs(aspect - desiredAspect) > 0) {
		if (aspect > desiredAspect) {
			mWindowOffset.Width = (mWindowSize.Width - desiredAspect * mWindowSize.Height) / 2.f;
		} else {
			mWindowOffset.Height = (mWindowSize.Height - mWindowSize.Width / desiredAspect) / 2.f;
		}
	}

	if (mSwapChain != nullptr) {
		// Resize swap chain buffers.
		check_hresult(mSwapChain->ResizeBuffers(
			2,
			lround(mWindowSize.Width),
			lround(mWindowSize.Height),
			DXGI_FORMAT_B8G8R8A8_UNORM,
			0)
		);

		// Construct a bitmap descriptor that is used with Direct2D rendering.
		D2D1_BITMAP_PROPERTIES1 properties{};
		properties.bitmapOptions |= D2D1_BITMAP_OPTIONS_TARGET;
		properties.bitmapOptions |= D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
		properties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
		properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
		properties.dpiX = 96.f;
		properties.dpiY = 96.f;

		// Query the DXGI version of the back buffer surface.
		com_ptr<IDXGISurface> dxgiBackBuffer;
		check_hresult(mSwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer)));

		// Create a new bitmap that's going to be used by the Direct2D.
		com_ptr<ID2D1Bitmap1> bitmap;
		check_hresult(m2DDeviceCtx->CreateBitmapFromDxgiSurface(
			dxgiBackBuffer.get(),
			&properties,
			bitmap.put()
		));

		// Assign the created bitmap as Direct2D render target.
		m2DDeviceCtx->SetTarget(bitmap.get());
	} else {
		// Query the underlying DXGI device from the Direct3D device.
		com_ptr<IDXGIDevice> dxgiDevice;
		check_bool(m3DDevice.try_as(dxgiDevice));

		// Query the underlying adapter (GPU/CPU) from the device.
		com_ptr<IDXGIAdapter> dxgiAdapter;
		check_hresult(dxgiDevice->GetAdapter(dxgiAdapter.put()));

		// Query the factory object that created the DXGI device.
		com_ptr<IDXGIFactory2> dxgiFactory;
		check_hresult(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory)));

		// Create and define a swap chain descriptor.
		DXGI_SWAP_CHAIN_DESC1 descriptor{};
		descriptor.Width = 0;
		descriptor.Height = 0;
		descriptor.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		descriptor.Stereo = false;
		descriptor.SampleDesc.Count = 1;  // disable multi-sampling
		descriptor.SampleDesc.Quality = 0;
		descriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		descriptor.BufferCount = 2;
		descriptor.Scaling = DXGI_SCALING_NONE;
		descriptor.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // FLIP mode is mandatory!
		descriptor.Flags = 0;

		// Create a swap chain for the window.
		check_hresult(dxgiFactory->CreateSwapChainForCoreWindow(
			m3DDevice.get(),
			get_unknown(mWindow.get()),
			&descriptor,
			nullptr, // allow on all displays
			mSwapChain.put()
		));

		// Construct a bitmap descriptor that is used with Direct2D rendering.
		D2D1_BITMAP_PROPERTIES1 properties{};
		properties.bitmapOptions |= D2D1_BITMAP_OPTIONS_TARGET;
		properties.bitmapOptions |= D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
		properties.pixelFormat.format = descriptor.Format;
		properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
		properties.dpiX = mDpi;
		properties.dpiY = mDpi;

		// Query the DXGI version of the back buffer surface.
		com_ptr<IDXGISurface> dxgiBackBuffer;
		check_hresult(mSwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer)));

		// Create a new bitmap that's going to be used by the Direct2D.
		com_ptr<ID2D1Bitmap1> bitmap;
		check_hresult(m2DDeviceCtx->CreateBitmapFromDxgiSurface(
			dxgiBackBuffer.get(),
			&properties,
			bitmap.put()
		));

		// Assign the created bitmap as Direct2D render target.
		m2DDeviceCtx->SetTarget(bitmap.get());
	}
}

void Renderer::setWindow(const ApplicationWindow& window) {
	OutputDebugStringA("GraphicsContext::setWindow\n");
	mWindow = window;
	mWindowSize = Size(window.Bounds().Width, window.Bounds().Height);
	mDpi = DisplayInformation::GetForCurrentView().LogicalDpi();
	m2DDeviceCtx->SetDpi(mDpi, mDpi);
	initWindowResources();
}

void Renderer::setWindowSize(const Size& size) {
	OutputDebugStringA("GraphicsContext::setWindowSize\n");
	if (mWindowSize != size) {
		mWindowSize = size;
		initWindowResources();
	}
}

void Renderer::setDpi(float dpi) {
	OutputDebugStringA("GraphicsContext::setDpi\n");
	if (mDpi != dpi) {
		mDpi = dpi;
		mWindowSize = Size(mWindow.get().Bounds().Width, mWindow.get().Bounds().Height);
		m2DDeviceCtx->SetDpi(mDpi, mDpi);
		initWindowResources();
	}
}

void Renderer::clear() {
	m2DDeviceCtx->BeginDraw();
	m2DDeviceCtx->Clear(D2D1::ColorF(D2D1::ColorF::Black));
}

void Renderer::present() {
	check_hresult(m2DDeviceCtx->EndDraw());
	auto presentResult = mSwapChain->Present(1, 0);

	// Recreate our resources whether the GPU was disconnected or went to a errorneous state.
	if (presentResult == DXGI_ERROR_DEVICE_REMOVED || presentResult == DXGI_ERROR_DEVICE_RESET) {
		initDeviceResources();
		initWindowResources();
	} else {
		check_hresult(presentResult);
	}
}

void Renderer::draw(com_ptr<ID2D1Brush> brush, Rectangle::Ref rect) {
	m2DDeviceCtx->FillRectangle({
	mWindowOffset.Width + (-rect->extent.x + rect->position.x) * (mWindowSize.Width - mWindowOffset.Width * 2),
	mWindowOffset.Height + (-rect->extent.y + rect->position.y) * (mWindowSize.Height - mWindowOffset.Height * 2),
	mWindowOffset.Width + (rect->extent.x + rect->position.x) * (mWindowSize.Width - mWindowOffset.Width * 2),
	mWindowOffset.Height + (rect->extent.y + rect->position.y) * (mWindowSize.Height - mWindowOffset.Height * 2),
		}, brush.get());
}

void Renderer::draw(com_ptr<ID2D1Brush> brush, const Text& text) {
	auto size = text.fontSize * (mWindowSize.Height - mWindowOffset.Height * 2.f);
	com_ptr<IDWriteTextFormat> format;
	mDWriteFactory->CreateTextFormat(
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
	auto x = mWindowOffset.Width + text.position.x * (mWindowSize.Width - mWindowOffset.Width * 2);
	auto y = mWindowOffset.Height + text.position.y * (mWindowSize.Height - mWindowOffset.Height * 2);
	m2DDeviceCtx->DrawText(
		text.text.c_str(),
		UINT32(text.text.size()),
		format.get(),
		{ x - text.text.length() * size * .5f,y,x + text.text.length() * size * .5f,y },
		brush.get()
	);
}