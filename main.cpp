#include <chrono>
#include <cwchar>
#include <d2d1_3.h>
#include <d3d11.h>
#include <dwrite.h>
#include <dxgi1_6.h>
#include <wrl.h>

using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::Gaming::Input;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;
using namespace Microsoft::WRL;

// =================
// === Constants ===
// =================

// The amount of update operations per second.
constexpr auto UPDATES_PER_SECOND = 25;
// The update physical step size in milliseconds.
constexpr auto UPDATE_MILLIS = 1000 / 25;

// =================
// === Utilities ===
// =================

// A utility function to get current time in milliseconds.
inline unsigned long CurrentMillis() {
	using namespace std::chrono;
	return (unsigned long)duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

// A utility function to throw an exception if HRESULT was failed.
inline void ThrowIfFailed(HRESULT hr) {
	if (FAILED(hr)) {
		throw Platform::Exception::CreateException(hr);
	}
}

// A utility function to convert device-independent pixels to physical pixels.
inline float ConvertDipsToPixels(float dips, float dpi) {
	static const float dipsPerInch = 96.0f;
	return floorf(dips * dpi / dipsPerInch + 0.5f);
}

// ============
// === Game ===
// ============

ref class Pong sealed : public IFrameworkView, IFrameworkViewSource
{
public:
	virtual IFrameworkView^ CreateView()
	{
		return ref new Pong();
	}

	virtual void Initialize(CoreApplicationView^ view)
	{
		view->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &Pong::Activated);
		Gamepad::GamepadAdded += ref new EventHandler<Gamepad^>(this, &Pong::GamepadAdded);
		Gamepad::GamepadRemoved += ref new EventHandler<Gamepad^>(this, &Pong::GamepadRemoved);
		InitializeGraphics();
	}

	void InitializeGraphics()
	{
		InitializeDirect3D();
		InitializeDirect2D();
		InitializeDirectWrite();
		InitializeBrushes();
	}

	void InitializeDirect3D()
	{
		// specify the desired additional behavior how the device will be created.
		UINT flags = 0;
		flags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT; // for Direct2D compatibility
		flags |= D3D11_CREATE_DEVICE_SINGLETHREADED;
		#ifdef _DEBUG
		flags |= D3D11_CREATE_DEVICE_DEBUG;
		#endif

		// specify the feature levels we want to support (ordering matters!).
		D3D_FEATURE_LEVEL featureLevels[] =
		{
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
			&m3dDevice,
			nullptr,
			&m3dCtx));
	}

	void InitializeDirect2D()
	{
		// specify creation configuration for a new Direct2D factory.
		D2D1_FACTORY_OPTIONS options;
		#ifdef _DEBUG
		options.debugLevel = D2D1_DEBUG_LEVEL_WARNING;
		#endif

		// construct a new Direct2D factory to build Direct2D resources.
		ThrowIfFailed(D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED,
			options,
			m2dFactory.GetAddressOf()
		));

		// query the underlying DXGI device from the Direct3D device.
		ComPtr<IDXGIDevice3> dxgiDevice;
		ThrowIfFailed(m3dDevice.As(&dxgiDevice));

		// create a Direct2D device and device context items.
		ThrowIfFailed(m2dFactory->CreateDevice(dxgiDevice.Get(), m2dDevice.GetAddressOf()));
		ThrowIfFailed(m2dDevice->CreateDeviceContext(
			D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
			&m2dCtx
		));

	}

	void InitializeDirectWrite()
	{
		// construct a new DirectWrite factory to build text resources.
		ThrowIfFailed(DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown**>(mWritefactory.GetAddressOf())
		));
	}

	void InitializeBrushes()
	{
		// create a white brush to be used when drawing white objects.
		ThrowIfFailed(m2dCtx->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::White),
			&mWhiteBrush
		));
	}

	virtual void SetWindow(CoreWindow^ window)
	{
		window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &Pong::WindowClosed);
		window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &Pong::WindowVisibilityChanged);
		window->SizeChanged += ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &Pong::WindowSizeChanged);
		ResizeContent();
	}

	virtual void Load(Platform::String^)
	{
		// ... no operations required
	}

	virtual void Run()
	{
		auto millisAccumulator = 0l;
		auto oldMillis = CurrentMillis();
		while (!mWindowClosed) {
			auto window = CoreWindow::GetForCurrentThread();
			if (mWindowVisible) {
				window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
				CheckInput();

				// calculate the time usable for the current frame.
				auto newMillis = CurrentMillis();
				auto delta = min(newMillis - oldMillis, 100ul);
				oldMillis = newMillis;
				millisAccumulator += delta;

				// perform ticking of the game logic and physics.
				while (millisAccumulator >= UPDATE_MILLIS) {
					Update(UPDATE_MILLIS);
					millisAccumulator -= UPDATE_MILLIS;
				}

				// perform interpolated rendering of the game scene.
				auto alpha = double(millisAccumulator) / double(UPDATE_MILLIS);
				Render(alpha);
			}
			else {
				window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
			}
		}
	}

	virtual void Uninitialize()
	{
		// ... no operations required
	}

	void WindowClosed(CoreWindow^, CoreWindowEventArgs^)
	{
		mWindowClosed = true;
	}

	void Activated(CoreApplicationView^, IActivatedEventArgs^)
	{
		CoreWindow::GetForCurrentThread()->Activate();
	}

	void WindowVisibilityChanged(CoreWindow^, VisibilityChangedEventArgs^ args)
	{
		mWindowVisible = args->Visible;
	}

	void WindowSizeChanged(CoreWindow^, WindowSizeChangedEventArgs^ args)
	{
		ResizeContent();
	}

	void ResizeContent()
	{
		auto window = CoreWindow::GetForCurrentThread();
		ResizeGameObjects(window);
		ResizeSwapchain(window);
		/*
		DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();
		auto dpi = currentDisplayInformation->LogicalDpi;

		auto bottom = CoreWindow::GetForCurrentThread()->Bounds.Bottom;
		auto bottom_pix = ConvertDipsToPixels(bottom, dpi);

		auto bounds = window->Bounds;
		auto wpix = ConvertDipsToPixels(bounds.Width, dpi);
		auto hpix = ConvertDipsToPixels(bounds.Height, dpi);

		wchar_t buffer[128];
		swprintf_s(buffer, 128, L"width: %.2f height: %.2f\n", windowSize.Width, windowSize.Height);
		OutputDebugString(buffer);
		// mBottomWallRect = { 0,windowSize.Height - (windowSize.Height / 10),windowSize.Width, windowSize.Height / 10 };
		*/
	}

	void ResizeGameObjects(CoreWindow^ window)
	{
		auto windowSize = Size(window->Bounds.Width, window->Bounds.Height);

		mTopWallRect.top = 0;
		mTopWallRect.bottom = windowSize.Height / 20;
		mTopWallRect.left = 0;
		mTopWallRect.right = windowSize.Width;
	}

	void ResizeSwapchain(CoreWindow^ window)
	{
		auto windowSize = Size(window->Bounds.Width, window->Bounds.Height);

		// release old render target if any.
		m2dCtx->SetTarget(nullptr);

		if (mSwapChain) {
			mSwapChain->ResizeBuffers(2, windowSize.Width, windowSize.Height, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
		} else {
			// query the DXGI factory from our DirectX 11 device.
			ComPtr<IDXGIDevice1> dxgiDevice;
			ThrowIfFailed(m3dDevice.As(&dxgiDevice));
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
				m3dDevice.Get(),
				reinterpret_cast<IUnknown*>(window),
				&desc,
				nullptr,
				&mSwapChain
			));
		}

		// construct a bitmap descriptor that is used with Direct2D rendering.
		D2D1_BITMAP_PROPERTIES1 properties = {};
		properties.bitmapOptions |= D2D1_BITMAP_OPTIONS_TARGET;
		properties.bitmapOptions |= D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
		properties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
		properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;

		// query the DXGI version of the back buffer surface.
		ComPtr<IDXGISurface> dxgiBackBuffer;
		ThrowIfFailed(mSwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer)));

		// create a new bitmap that's going to be used by the Direct2D.
		ComPtr<ID2D1Bitmap1> bitmap;
		ThrowIfFailed(m2dCtx->CreateBitmapFromDxgiSurface(
			dxgiBackBuffer.Get(),
			&properties,
			&bitmap
		));

		// assign the created bitmap as Direct2D render target.
		m2dCtx->SetTarget(bitmap.Get());
	}

	void GamepadAdded(Object^ o, Gamepad^ gamepad)
	{
		// TODO
	}

	void GamepadRemoved(Object^ o, Gamepad^ gamepad)
	{
		// TODO
	}

	void CheckInput()
	{
		// TODO
	}

	void Update(int dt)
	{
		// TODO
	}

	void Render(double alpha)
	{
		m2dCtx->BeginDraw();
		m2dCtx->Clear(D2D1::ColorF(D2D1::ColorF::DarkBlue));

		m2dCtx->FillRectangle(mTopWallRect, mWhiteBrush.Get());
		// m2dCtx->FillRectangle(mBottomWallRect, mWhiteBrush.Get());
		
		ThrowIfFailed(m2dCtx->EndDraw());
		ThrowIfFailed(mSwapChain->Present(1, 0));
	}
private:
	// The flag used to stop execution of the application's main loop when the main window is closed.
	bool mWindowClosed;
	// The flag used to indicate whether the main window is visible and game should be rendered.
	bool mWindowVisible;

	ComPtr<ID3D11Device>		m3dDevice;
	ComPtr<ID3D11DeviceContext>	m3dCtx;
	ComPtr<ID2D1Factory6>		m2dFactory;
	ComPtr<ID2D1Device5>		m2dDevice;
	ComPtr<ID2D1DeviceContext5>	m2dCtx;
	ComPtr<IDXGISwapChain1>		mSwapChain;
	ComPtr<IDWriteFactory>		mWritefactory;

	ComPtr<ID2D1SolidColorBrush> mWhiteBrush;

	D2D1_RECT_F mTopWallRect;
	D2D1_RECT_F mBottomWallRect;
};

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	CoreApplication::Run(ref new Pong());
	return 0;
}
