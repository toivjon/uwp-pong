#pragma once

#include <d2d1.h>
#include <d2d1_3.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <memory>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Core.h>

#include "vec2f.hpp"

// An alias for the CoreWindow to avoid using the full name monster.
using ApplicationWindow = winrt::Windows::UI::Core::CoreWindow;

enum class ObjectID { NONE, LEFT_PADDLE, RIGHT_PADDLE, BALL, TOP_WALL, BOTTOM_WALL, LEFT_GOAL, RIGHT_GOAL };

struct Rectangle {
	ObjectID id;
	Vec2f	 extent = { 0.f,0.f };
	Vec2f	 position = { 0.f,0.f };
	Vec2f	 velocity = { 0.f, 0.f };
};

struct Text {
	Vec2f		 position = { 0.f,0.f };
	std::wstring text;
	float		 fontSize = 0.f;
};

class Renderer {
public:
	using Ptr = std::unique_ptr<Renderer>;

	Renderer();

	void initDeviceResources();
	void initWindowResources();

	void setWindow(const ApplicationWindow& window);
	void setWindowSize(const winrt::Windows::Foundation::Size& size);
	void setDpi(float dpi);

	void clear();
	void present();

	winrt::com_ptr<ID2D1DeviceContext5> getD2DContext() const { return m2DDeviceCtx; }
	winrt::Windows::Foundation::Size getWindowSize() const { return mWindowSize; }
	winrt::Windows::Foundation::Size getWindowOffset() const { return mWindowOffset; }
	winrt::com_ptr<IDWriteFactory3> getDWriteFactory() const { return mDWriteFactory; }
	winrt::com_ptr<ID2D1Brush> getWhiteBrush() const { return mWhiteBrush; }
	winrt::com_ptr<ID2D1Brush> getBlackBrush() const { return mBlackBrush; }

	void draw(winrt::com_ptr<ID2D1Brush> brush, const Rectangle& rect);
	void draw(winrt::com_ptr<ID2D1Brush> brush, const Text& text);

private:
	winrt::agile_ref<ApplicationWindow>  mWindow;
	winrt::Windows::Foundation::Size	 mWindowSize;
	winrt::Windows::Foundation::Size	 mWindowOffset = { 0,0 };
	float								 mDpi = 0.f;

	winrt::com_ptr<ID3D11Device>		 m3DDevice;
	winrt::com_ptr<ID3D11DeviceContext>  m3DDeviceCtx;
	winrt::com_ptr<ID2D1Factory6>		 m2DFactory;
	winrt::com_ptr<ID2D1Device5>		 m2DDevice;
	winrt::com_ptr<ID2D1DeviceContext5>  m2DDeviceCtx;
	winrt::com_ptr<IDXGISwapChain1>		 mSwapChain;
	winrt::com_ptr<IDWriteFactory3>		 mDWriteFactory;
	winrt::com_ptr<ID2D1SolidColorBrush> mWhiteBrush;
	winrt::com_ptr<ID2D1SolidColorBrush> mBlackBrush;
};