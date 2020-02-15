#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl.h>

namespace pong
{
	class Graphics final
	{
	public:
		Graphics();

		void SetWindow(Windows::UI::Core::CoreWindow^ window);
		void Present();
	private:
		Microsoft::WRL::ComPtr<ID3D11Device> mDevice;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> mDeviceCtx;
		Microsoft::WRL::ComPtr<IDXGISwapChain1> mSwapChain;
	};
}