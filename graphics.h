#pragma once

#include <d3d11.h>
#include <d2d1_3.h>
#include <dwrite.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace pong
{
	class Graphics final
	{
	public:
		Graphics();

		void SetWindow(Windows::UI::Core::CoreWindow^ window);

		void BeginDraw();
		void EndDraw();

		Microsoft::WRL::ComPtr<ID2D1DeviceContext5> Get2DContext() { return m2DDeviceCtx; }
		Microsoft::WRL::ComPtr<IDWriteTextFormat> GetTextFormat(int size);

		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> GetWhiteBrush() { return mWhiteBrush; }
	private:
		void InitD3DContext();
		void InitD2DContext();
		void InitDirectWrite();
	private:
		Microsoft::WRL::ComPtr<ID2D1Factory6>			m2DFactory;
		Microsoft::WRL::ComPtr<ID2D1Device5>			m2DDevice;
		Microsoft::WRL::ComPtr<ID2D1DeviceContext5>		m2DDeviceCtx;
		Microsoft::WRL::ComPtr<ID3D11Device>			m3DDevice;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext>		m3DDeviceCtx;
		Microsoft::WRL::ComPtr<IDXGISwapChain1>			mSwapChain;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	mRenderTargetView;
		Microsoft::WRL::ComPtr<IDWriteFactory>			mWritefactory;
		Microsoft::WRL::ComPtr<IDWriteTextFormat>		mTextFormat;

		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> mWhiteBrush;
	};
}