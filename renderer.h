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

private:
	winrt::agile_ref<ApplicationWindow> mWindow;
	winrt::Windows::Foundation::Size	mWindowSize;
	float								mDpi;

	winrt::com_ptr<ID3D11Device>		m3DDevice;
	winrt::com_ptr<ID3D11DeviceContext> m3DDeviceCtx;
	winrt::com_ptr<ID2D1Factory6>		m2DFactory;
	winrt::com_ptr<ID2D1Device5>		m2DDevice;
	winrt::com_ptr<ID2D1DeviceContext5> m2DDeviceCtx;
	winrt::com_ptr<IDXGISwapChain1>		mSwapChain;
};