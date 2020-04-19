#pragma once

#include <d2d1_3.h>
#include <d3d11.h>
#include <dwrite.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace pong::graphics
{
	class Graphics final
	{
	public:
		Graphics();
		Microsoft::WRL::ComPtr<ID2D1DeviceContext5> GetD2DDeviceCtx() { return mD2DDeviceCtx; }
		Microsoft::WRL::ComPtr<ID3D11Device> GetD3DDevice() { return mD3DDevice; }
		Microsoft::WRL::ComPtr<IDWriteFactory> GetWriteFactory() { return mDWritefactory; }
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> GetWhiteBrush() { return mWhiteBrush; }
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> GetBlackBrush() { return mBlackBrush; }
	private:
		void InitDirect3D();
		void InitDirect2D();
		void InitDirectWrite();
		void InitBrushes();
	private:
		Microsoft::WRL::ComPtr<ID3D11Device>		mD3DDevice;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext>	mD3DDeviceCtx;

		Microsoft::WRL::ComPtr<ID2D1Factory6>		mD2DFactory;
		Microsoft::WRL::ComPtr<ID2D1Device5>		mD2DDevice;
		Microsoft::WRL::ComPtr<ID2D1DeviceContext5>	mD2DDeviceCtx;

		Microsoft::WRL::ComPtr<IDWriteFactory> mDWritefactory;

		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> mWhiteBrush;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> mBlackBrush;
	};
}