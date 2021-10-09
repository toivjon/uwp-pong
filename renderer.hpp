#pragma once

#include <d2d1.h>
#include <d2d1_3.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <memory>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Core.h>

// An alias for the CoreWindow to avoid using the full name monster.
using ApplicationWindow = winrt::Windows::UI::Core::CoreWindow;

// Vec2f represents a 2D vector with floating point values.
struct Vec2f final {
	auto operator+(const Vec2f& v) const -> Vec2f { return { x + v.x, y + v.y }; }
	auto operator-(const Vec2f& v) const -> Vec2f { return { x - v.x, y - v.y }; }

	void operator+=(const Vec2f& v) { x += v.x; y += v.y; }
	void operator-=(const Vec2f& v) { x -= v.x; y -= v.y; }

	auto operator*(float s) const -> Vec2f { return { x * s, y * s }; }

	float x;
	float y;
};

struct Rectangle {
	using Ref = std::shared_ptr<Rectangle>;
	Vec2f	 extent = { 0.f,0.f };
	Vec2f	 position = { 0.f,0.f };
	Vec2f	 velocity = { 0.f, 0.f };
};

struct Text {
	using Ref = std::shared_ptr<Text>;
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

	winrt::com_ptr<ID2D1Brush> getWhiteBrush() const { return mWhiteBrush; }
	winrt::com_ptr<ID2D1Brush> getBlackBrush() const { return mBlackBrush; }

	void draw(winrt::com_ptr<ID2D1Brush> brush, Rectangle::Ref rect);
	void draw(winrt::com_ptr<ID2D1Brush> brush, Text::Ref text);

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