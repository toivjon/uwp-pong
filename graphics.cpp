#include "graphics.h"
#include "util.h"

using namespace Microsoft::WRL;
using namespace pong;
using namespace Windows::UI::Core;

Graphics::Graphics()
{
	InitD3DContext();
	InitD2DContext();
}

void Graphics::SetWindow(CoreWindow^ window)
{
	// query the DXGI factory from our DirectX 11 device.
	ComPtr<IDXGIDevice1> dxgiDevice;
	ThrowIfFailed(m3DDevice.As(&dxgiDevice));
	ComPtr<IDXGIAdapter> dxgiAdapter;
	ThrowIfFailed(dxgiDevice->GetAdapter(&dxgiAdapter));
	ComPtr<IDXGIFactory2> dxgiFactory;
	dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), &dxgiFactory);

	// specify swap chain configuration.
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = 2;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	desc.SampleDesc.Count = 1;

	// create the swap chain.
	ThrowIfFailed(dxgiFactory->CreateSwapChainForCoreWindow(
		m3DDevice.Get(),
		reinterpret_cast<IUnknown*>(window),
		&desc,
		nullptr,
		&mSwapChain
	));

	// construct a bitmap descriptor that is used with Direct2D rendering.
	D2D1_BITMAP_PROPERTIES1 properties = {};
	properties.bitmapOptions |= D2D1_BITMAP_OPTIONS_TARGET;
	properties.bitmapOptions |= D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
	properties.pixelFormat.format = desc.Format;
	properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;

	// query the DXGI version of the back buffer surface.
	ComPtr<IDXGISurface> dxgiBackBuffer;
	ThrowIfFailed(mSwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer)));

	// create a new bitmap that's going to be used by the Direct2D.
	ComPtr<ID2D1Bitmap1> bitmap;
	ThrowIfFailed(m2DDeviceCtx->CreateBitmapFromDxgiSurface(
		dxgiBackBuffer.Get(),
		&properties,
		&bitmap
	));

	// assign the created bitmap as Direct2D render target.
	m2DDeviceCtx->SetTarget(bitmap.Get());
}

void Graphics::BeginDraw()
{
	m2DDeviceCtx->BeginDraw();
	m2DDeviceCtx->Clear(D2D1::ColorF(D2D1::ColorF::DarkBlue));
}

void Graphics::EndDraw()
{
	ThrowIfFailed(m2DDeviceCtx->EndDraw());
	ThrowIfFailed(mSwapChain->Present(1, 0));
}

void Graphics::InitD3DContext()
{
	// specify the desired additional behavior how the device will be created.
	UINT flags = 0;
	flags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT; // for Direct2D compatibility
	flags |= D3D11_CREATE_DEVICE_SINGLETHREADED;
	#ifdef _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif

	// specify the feature levels we want to support (ordering matters!).
	D3D_FEATURE_LEVEL featureLevels[] =
	{
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
		&m3DDevice,
		nullptr,
		&m3DDeviceCtx));
}


void Graphics::InitD2DContext()
{
	// specify creation configuration for a new Direct2D factory.
	D2D1_FACTORY_OPTIONS options;
	#ifdef _DEBUG
	options.debugLevel = D2D1_DEBUG_LEVEL_WARNING;
	#endif

	// construct a new Direct2D factory to build Direct2D resources.
	ThrowIfFailed(D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		options,
		m2DFactory.GetAddressOf()
	));

	// query the underlying DXGI device from the Direct3D device.
	ComPtr<IDXGIDevice3> dxgiDevice;
	ThrowIfFailed(m3DDevice.As(&dxgiDevice));

	// create a Direct2D device and device context items.
	ThrowIfFailed(m2DFactory->CreateDevice(dxgiDevice.Get(), m2DDevice.GetAddressOf()));
	ThrowIfFailed(m2DDevice->CreateDeviceContext(
		D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
		&m2DDeviceCtx
	));
}