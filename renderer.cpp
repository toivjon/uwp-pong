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
	// Specify the desired additional behavior how the Direct2D factory will be created.
	D2D1_FACTORY_OPTIONS options{};
	#if defined(_DEBUG)
	options.debugLevel = D2D1_DEBUG_LEVEL_WARNING;
	#endif

	// Construct a new Direct2D factory to build Direct2D resources.
	check_hresult(D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		options,
		d2dFactory.put()
	));

	// Construct a new DirectDraw factory to build DirectDraw resources.
	check_hresult(DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory3),
		reinterpret_cast<::IUnknown**>(dWriteFactory.put())
	));

	initDeviceResources();

	// Build the white and black brush, which are the only brushes we need.
	check_hresult(d2dDeviceCtx->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), whiteBrush.put()));
	check_hresult(d2dDeviceCtx->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), blackBrush.put()));
}

void Renderer::initDeviceResources() {
	// clear all possible old definitions.
	swapChain = nullptr;
	d2dDeviceCtx = nullptr;
	d2dDevice = nullptr;
	d3dDeviceCtx = nullptr;
	d3dDevice = nullptr;

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
		d3dDevice.put(),
		nullptr,
		d3dDeviceCtx.put()
	));

	// Query and use the underlying DXGI device to create a Direct2D device.
	com_ptr<IDXGIDevice3> dxgiDevice;
	check_bool(d3dDevice.try_as(dxgiDevice));
	check_hresult(d2dFactory->CreateDevice(dxgiDevice.get(), d2dDevice.put()));

	// Construct a new Direct2D device context.
	check_hresult(d2dDevice->CreateDeviceContext(
		D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
		d2dDeviceCtx.put()
	));
}

void Renderer::initWindowResources() {
	d2dDeviceCtx->SetTarget(nullptr);
	d3dDeviceCtx->Flush();

	// TODO perhaps we could adjust this in some other way?
	// calculate window entity offset to maintain aspect ratio.
	windowOffset = { 0,0 };
	auto aspect = windowSize.Width / windowSize.Height;
	const auto desiredAspect = 1.3f;
	if (abs(aspect - desiredAspect) > 0) {
		if (aspect > desiredAspect) {
			windowOffset.Width = (windowSize.Width - desiredAspect * windowSize.Height) / 2.f;
		} else {
			windowOffset.Height = (windowSize.Height - windowSize.Width / desiredAspect) / 2.f;
		}
	}

	if (swapChain != nullptr) {
		// Resize swap chain buffers.
		check_hresult(swapChain->ResizeBuffers(
			2,
			lround(windowSize.Width),
			lround(windowSize.Height),
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
		check_hresult(swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer)));

		// Create a new bitmap that's going to be used by the Direct2D.
		com_ptr<ID2D1Bitmap1> bitmap;
		check_hresult(d2dDeviceCtx->CreateBitmapFromDxgiSurface(
			dxgiBackBuffer.get(),
			&properties,
			bitmap.put()
		));

		// Assign the created bitmap as Direct2D render target.
		d2dDeviceCtx->SetTarget(bitmap.get());
	} else {
		// Query the underlying DXGI device from the Direct3D device.
		com_ptr<IDXGIDevice> dxgiDevice;
		check_bool(d3dDevice.try_as(dxgiDevice));

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
			d3dDevice.get(),
			get_unknown(window.get()),
			&descriptor,
			nullptr, // allow on all displays
			swapChain.put()
		));

		// Construct a bitmap descriptor that is used with Direct2D rendering.
		D2D1_BITMAP_PROPERTIES1 properties{};
		properties.bitmapOptions |= D2D1_BITMAP_OPTIONS_TARGET;
		properties.bitmapOptions |= D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
		properties.pixelFormat.format = descriptor.Format;
		properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
		properties.dpiX = dpi;
		properties.dpiY = dpi;

		// Query the DXGI version of the back buffer surface.
		com_ptr<IDXGISurface> dxgiBackBuffer;
		check_hresult(swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer)));

		// Create a new bitmap that's going to be used by the Direct2D.
		com_ptr<ID2D1Bitmap1> bitmap;
		check_hresult(d2dDeviceCtx->CreateBitmapFromDxgiSurface(
			dxgiBackBuffer.get(),
			&properties,
			bitmap.put()
		));

		// Assign the created bitmap as Direct2D render target.
		d2dDeviceCtx->SetTarget(bitmap.get());
	}
}

void Renderer::setWindow(const ApplicationWindow& window) {
	this->window = window;
	windowSize = Size(window.Bounds().Width, window.Bounds().Height);
	dpi = DisplayInformation::GetForCurrentView().LogicalDpi();
	d2dDeviceCtx->SetDpi(dpi, dpi);
	initWindowResources();
}

void Renderer::setWindowSize(const Size& size) {
	if (windowSize != size) {
		windowSize = size;
		initWindowResources();
	}
}

void Renderer::setDpi(float dpi) {
	if (dpi != dpi) {
		dpi = dpi;
		windowSize = Size(window.get().Bounds().Width, window.get().Bounds().Height);
		d2dDeviceCtx->SetDpi(dpi, dpi);
		initWindowResources();
	}
}

void Renderer::clear() {
	d2dDeviceCtx->BeginDraw();
	d2dDeviceCtx->Clear(D2D1::ColorF(D2D1::ColorF::Black));
}

void Renderer::present() {
	check_hresult(d2dDeviceCtx->EndDraw());
	auto presentResult = swapChain->Present(1, 0);

	// Recreate our resources whether the GPU was disconnected or went to a errorneous state.
	if (presentResult == DXGI_ERROR_DEVICE_REMOVED || presentResult == DXGI_ERROR_DEVICE_RESET) {
		initDeviceResources();
		initWindowResources();
	} else {
		check_hresult(presentResult);
	}
}

void Renderer::draw(com_ptr<ID2D1Brush> brush, const Rectangle& rect) const {
	d2dDeviceCtx->FillRectangle({
	windowOffset.Width + (-rect.extent.x + rect.position.x) * (windowSize.Width - windowOffset.Width * 2),
	windowOffset.Height + (-rect.extent.y + rect.position.y) * (windowSize.Height - windowOffset.Height * 2),
	windowOffset.Width + (rect.extent.x + rect.position.x) * (windowSize.Width - windowOffset.Width * 2),
	windowOffset.Height + (rect.extent.y + rect.position.y) * (windowSize.Height - windowOffset.Height * 2),
		}, brush.get());
}

void Renderer::draw(com_ptr<ID2D1Brush> brush, const Text& text) const {
	auto size = text.fontSize * (windowSize.Height - windowOffset.Height * 2.f);
	com_ptr<IDWriteTextFormat> format;
	dWriteFactory->CreateTextFormat(
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
	auto x = windowOffset.Width + text.position.x * (windowSize.Width - windowOffset.Width * 2);
	auto y = windowOffset.Height + text.position.y * (windowSize.Height - windowOffset.Height * 2);
	d2dDeviceCtx->DrawText(
		text.text.c_str(),
		UINT32(text.text.size()),
		format.get(),
		{ x - text.text.length() * size * .5f,y,x + text.text.length() * size * .5f,y },
		brush.get()
	);
}