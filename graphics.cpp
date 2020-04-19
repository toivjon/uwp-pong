#include "graphics.h"

#include <cassert>

using namespace pong::graphics;
using namespace Microsoft::WRL;
using namespace D2D1;

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