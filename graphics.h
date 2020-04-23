#pragma once

#include "geometry.h"

#include <d2d1_3.h>
#include <d3d11.h>
#include <dwrite.h>
#include <dxgi1_6.h>
#include <string>
#include <vector>
#include <wrl.h>

namespace pong::graphics
{
	// 0.75f
	// 6
	// 3
	class Graphics final
	{
	public:
		Graphics();
		void SetCoreWindow(Windows::UI::Core::CoreWindow^ window);
		void BeginDrawAndClear();
		void EndDrawAndPresent();
		void DrawWhiteRects(const std::vector<geometry::Rectangle> rects);
		void DrawWhiteRect(const D2D1_RECT_F& rect);
		void DrawWhiteBigText(const std::wstring& text, const D2D1_RECT_F& rect);
		void DrawBlackSmallText(const std::wstring& text, const D2D1_RECT_F& rect);
		void DrawBlackMediumText(const std::wstring& text, const D2D1_RECT_F& rect);
	private:
		void InitDirect3D();
		void InitDirect2D();
		void InitDirectWrite();
		void InitBrushes();
		void BuildTextFormat(float size, Microsoft::WRL::ComPtr<IDWriteTextFormat>& format);
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

		Microsoft::WRL::ComPtr<IDWriteTextFormat> mSmallTextFormat;
		Microsoft::WRL::ComPtr<IDWriteTextFormat> mMediumTextFormat;
		Microsoft::WRL::ComPtr<IDWriteTextFormat> mBigTextFormat;
	};
}