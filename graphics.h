#pragma once

#include <d2d1_3.h>
#include <d3d11.h>
#include <dwrite.h>
#include <dxgi1_6.h>
#include <string>
#include <wrl.h>

namespace pong::graphics
{
	class Graphics final
	{
	public:
		Graphics();
		void SetCoreWindow(Windows::UI::Core::CoreWindow^ window);
		void BeginDrawAndClear();
		void EndDrawAndPresent();
		void FillWhiteRect(const D2D1_RECT_F& rect);
		void DrawWhiteText(const std::wstring& text, const D2D1_RECT_F& rect, Microsoft::WRL::ComPtr<IDWriteTextFormat> format);
		void DrawBlackText(const std::wstring& text, const D2D1_RECT_F& rect, Microsoft::WRL::ComPtr<IDWriteTextFormat> format);
		Microsoft::WRL::ComPtr<ID2D1DeviceContext5> GetD2DDeviceCtx() { return mD2DDeviceCtx; }
		Microsoft::WRL::ComPtr<IDWriteFactory> GetWriteFactory() { return mDWritefactory; }
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

		Microsoft::WRL::ComPtr<IDXGISwapChain1>	mSwapChain;
	};
}