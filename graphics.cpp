#include "graphics.h"
#include "util.h"

using namespace Microsoft::WRL;
using namespace pong;
using namespace Windows::UI::Core;

Graphics::Graphics()
{
	// create a DirectX 11 device item.
	ThrowIfFailed(D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&mDevice,
		nullptr,
		&mDeviceCtx));
}

void Graphics::SetWindow(CoreWindow^ window)
{
	// query the DXGI factory from our DirectX 11 device.
	ComPtr<IDXGIDevice1> dxgiDevice;
	ThrowIfFailed(mDevice.As(&dxgiDevice));
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
		mDevice.Get(),
		reinterpret_cast<IUnknown*>(window),
		&desc,
		nullptr,
		&mSwapChain
	));
}

void Graphics::Present()
{
	mSwapChain->Present(1, 0);
}