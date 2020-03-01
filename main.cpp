#include <chrono>
#include <concrt.h>
#include <cwchar>
#include <d2d1_3.h>
#include <d3d11.h>
#include <dwrite.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <string>
#include <wrl.h>

using namespace concurrency;
using namespace DirectX;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::Gaming::Input;
using namespace Windows::Graphics::Display;
using namespace Windows::System;
using namespace Windows::UI::Core;
using namespace Microsoft::WRL;

// =================
// === Constants ===
// =================

// The amount of update operations per second.
constexpr auto UPDATES_PER_SECOND = 25;
// The update physical step size in milliseconds.
constexpr auto UPDATE_MILLIS = 1000 / 25;
// The amount of dots in the center line.
constexpr auto CENTERLINE_DOTS = 15;
// The placeholder for the left player name.
constexpr const wchar_t* LEFT_PLAYER_NAME_PLACEHOLDER = L"player-1";
// The placeholder name for the right player name.
constexpr const wchar_t* RIGHT_PLAYER_NAME_PLACEHOLDER = L"player-2";

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

inline D2D1_RECT_F Interpolate(const D2D1_RECT_F& a, const D2D1_RECT_F& b, float alpha)
{
	D2D1_RECT_F result;
	result.top = a.top + (b.top - a.top) * alpha;
	result.bottom = a.bottom + (b.bottom - a.bottom) * alpha;
	result.left = a.left + (b.left - a.left) * alpha;
	result.right = a.right + (b.right - a.right) * alpha;
	return result;
}

// A utility function to perform a simple AABB intersection test.
inline bool Collides(const D2D1_RECT_F& a, const D2D1_RECT_F& b)
{
	return !(
		a.right  < b.left  ||
		a.bottom < b.top   ||
		a.left   > b.right ||
		a.top    > b.bottom);
}

// A utility to check whether target rect contains the target point.
inline bool Contains(const D2D1_RECT_F& rect, float x, float y) {
	return rect.left <= x && x <= rect.right
		&& rect.top  <= y && y <= rect.bottom;
}

// A utility to check whether a-rect contains the target b-rect.
inline bool Contains(const D2D1_RECT_F& a, const D2D1_RECT_F& b) {
	return Contains(a, b.left, b.top)
		&& Contains(a, b.left, b.bottom)
		&& Contains(a, b.right, b.top)
		&& Contains(a, b.right, b.bottom);
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
		InitializeGame();
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

		// create a black brush to be used when drawing black objects.
		ThrowIfFailed(m2dCtx->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::Black),
			&mBlackBrush
		));
	}

	void InitializeGame()
	{
		mLeftPlayerName = LEFT_PLAYER_NAME_PLACEHOLDER;
		mRightPlayerName = RIGHT_PLAYER_NAME_PLACEHOLDER;

		mBallDirection = XMVectorSet(1.f, 1.f, 0.f, 0.f);
		mBallDirection = XMVector2Normalize(mBallDirection);
	}

	void KeyDown(CoreWindow^ window, KeyEventArgs^ args)
	{
		switch (args->VirtualKey) {
		case VirtualKey::Up:
			mRightPaddleVelocity = -mPaddleVelocity;
			break;
		case VirtualKey::Down:
			mRightPaddleVelocity = mPaddleVelocity;
			break;
		case VirtualKey::W:
			mLeftPaddleVelocity = -mPaddleVelocity;
			break;
		case VirtualKey::S:
			mLeftPaddleVelocity = mPaddleVelocity;
			break;
		}
	}

	void KeyUp(CoreWindow^ window, KeyEventArgs^ args)
	{
		switch (args->VirtualKey) {
		case VirtualKey::Up:
			if (mRightPaddleVelocity < 0.f) {
				mRightPaddleVelocity = 0.f;
			}
			break;
		case VirtualKey::Down:
			if (mRightPaddleVelocity > 0.f) {
				mRightPaddleVelocity = 0.f;
			}
			break;
		case VirtualKey::W:
			if (mLeftPaddleVelocity < 0.f) {
				mLeftPaddleVelocity = 0.f;
			}
			break;
		case VirtualKey::S:
			if (mLeftPaddleVelocity > 0.f) {
				mLeftPaddleVelocity = 0.f;
			}
			break;
		}
	}

	virtual void SetWindow(CoreWindow^ window)
	{
		window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &Pong::WindowClosed);
		window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &Pong::WindowVisibilityChanged);
		window->SizeChanged += ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &Pong::WindowSizeChanged);
		window->KeyDown += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &Pong::KeyDown);
		window->KeyUp += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &Pong::KeyUp);
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
				float alpha = float(millisAccumulator) / float(UPDATE_MILLIS);
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
	}

	void ResizeGameObjects(CoreWindow^ window)
	{
		// get the width and height of the window.
		auto windowWidth = window->Bounds.Width;
		auto windowHeight = window->Bounds.Height;

		// calculate aspect ratio correction values.
		static const auto aspectRatio = (800.f / 600.f);
		auto currentAspectRatio = (windowWidth / windowHeight);
		auto widthSpacing = 0.f, heightSpacing = 0.f;
		if (currentAspectRatio > aspectRatio) {
			auto widthEdge = windowHeight * aspectRatio;
			widthSpacing = windowWidth - widthEdge;
		} else if (currentAspectRatio < aspectRatio) {
			auto heightEdge = windowWidth / aspectRatio;
			heightSpacing = windowHeight - heightEdge;
		}

		// calculate physical coefficients.
		static const float EPSILON = 0.001f;
		mPaddleVelocity = (windowHeight - heightSpacing) / 30;
		mBallVelocity = mPaddleVelocity / 4;

		// correct left paddle velocity if currently applied.
		if (mLeftPaddleVelocity > EPSILON || mLeftPaddleVelocity < -EPSILON) {
			mLeftPaddleVelocity = mPaddleVelocity;
		}

		// correct right paddle velocity if currently applied.
		if (mRightPaddleVelocity > EPSILON || mRightPaddleVelocity < -EPSILON) {
			mRightPaddleVelocity = mPaddleVelocity;
		}

		// calculate cell size and the view center points.
		auto cellSize = (windowHeight - heightSpacing) / 30;
		auto horizontalCenter = windowWidth / 2;
		auto verticalCenter = windowHeight / 2;

		mTopWallRect.top = heightSpacing / 2;
		mTopWallRect.bottom = mTopWallRect.top + cellSize;
		mTopWallRect.left = widthSpacing / 2;
		mTopWallRect.right = windowWidth - widthSpacing / 2;

		mBottomWallRect.top = windowHeight - heightSpacing / 2 - cellSize;
		mBottomWallRect.bottom = mBottomWallRect.top + cellSize;
		mBottomWallRect.left = widthSpacing / 2;
		mBottomWallRect.right = windowWidth - widthSpacing / 2;

		ThrowIfFailed(mWritefactory->CreateTextFormat(
			L"Calibri",
			nullptr,
			DWRITE_FONT_WEIGHT_REGULAR,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			cellSize * 6,
			L"en-us",
			&mPointsTextFormat
		));
		ThrowIfFailed(mPointsTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
		ThrowIfFailed(mPointsTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));

		mLeftPointsRect.top = heightSpacing / 2 + cellSize * 2;
		mLeftPointsRect.bottom = mLeftPointsRect.top + cellSize * 6;
		mLeftPointsRect.left = horizontalCenter - cellSize * 5;
		mLeftPointsRect.right = mLeftPointsRect.left + cellSize;

		mRightPointsRect.top = heightSpacing / 2 + cellSize * 2;
		mRightPointsRect.bottom = mRightPointsRect.top + cellSize * 6;
		mRightPointsRect.left = horizontalCenter + cellSize * 4;
		mRightPointsRect.right = mRightPointsRect.left + cellSize;

		ThrowIfFailed(mWritefactory->CreateTextFormat(
			L"Calibri",
			nullptr,
			DWRITE_FONT_WEIGHT_REGULAR,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			cellSize * 0.75f,
			L"en-us",
			&mLeftPlayerNameTextFormat
		));
		ThrowIfFailed(mLeftPlayerNameTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
		ThrowIfFailed(mLeftPlayerNameTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));

		ThrowIfFailed(mWritefactory->CreateTextFormat(
			L"Calibri",
			nullptr,
			DWRITE_FONT_WEIGHT_REGULAR,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			cellSize * 0.75f,
			L"en-us",
			&mRightPlayerNameTextFormat
		));
		ThrowIfFailed(mRightPlayerNameTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING));
		ThrowIfFailed(mRightPlayerNameTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));

		mLeftPlayerNameRect.top = heightSpacing / 2;
		mLeftPlayerNameRect.bottom = mLeftPlayerNameRect.top + cellSize;
		mLeftPlayerNameRect.left = widthSpacing / 2 + cellSize * .5f;
		mLeftPlayerNameRect.right = horizontalCenter - cellSize * 3;

		mRightPlayerNameRect.top = heightSpacing / 2;
		mRightPlayerNameRect.bottom = mRightPlayerNameRect.top + cellSize;
		mRightPlayerNameRect.left = horizontalCenter + cellSize * 3;
		mRightPlayerNameRect.right = windowWidth - widthSpacing / 2 - cellSize * .5f;

		for (auto i = 0; i < CENTERLINE_DOTS; i++) {
			mCenterlineRects[i].top = heightSpacing / 2 + (i*2 + 0.5f) * cellSize;
			mCenterlineRects[i].bottom = mCenterlineRects[i].top + cellSize;
			mCenterlineRects[i].left = horizontalCenter - (cellSize / 2);
			mCenterlineRects[i].right = mCenterlineRects[i].left + cellSize;
		}

		for (auto i = 0; i < 2; i++) {
			mLeftPaddleRects[i].top = verticalCenter - (2.5f * cellSize);
			mLeftPaddleRects[i].bottom = mLeftPaddleRects[i].top + 5 * cellSize;
			mLeftPaddleRects[i].left = widthSpacing / 2 + cellSize;
			mLeftPaddleRects[i].right = mLeftPaddleRects[i].left + cellSize;

			mRightPaddleRects[i].top = verticalCenter - (2.5f * cellSize);
			mRightPaddleRects[i].bottom = mRightPaddleRects[i].top + 5 * cellSize;
			mRightPaddleRects[i].left = windowWidth - (2 * cellSize + widthSpacing / 2);
			mRightPaddleRects[i].right = mRightPaddleRects[i].left + cellSize;

			mBallRects[i].top = verticalCenter - (.5f * cellSize);
			mBallRects[i].bottom = mBallRects[i].top + cellSize;
			mBallRects[i].left = horizontalCenter - (.5f * cellSize);
			mBallRects[i].right = mBallRects[i].left + cellSize;
		}

		mLeftGoalRect.top = 0;
		mLeftGoalRect.bottom = windowHeight;
		mLeftGoalRect.left = -D3D10_FLOAT32_MAX;
		mLeftGoalRect.right = widthSpacing / 2;

		mRightGoalRect.top = 0;
		mRightGoalRect.bottom = windowHeight;
		mRightGoalRect.left = windowWidth - widthSpacing / 2;
		mRightGoalRect.right = D3D10_FLOAT32_MAX;
	}

	void ResizeSwapchain(CoreWindow^ window)
	{
		auto windowSize = Size(window->Bounds.Width, window->Bounds.Height);

		// release old render target if any.
		m2dCtx->SetTarget(nullptr);

		if (mSwapChain) {
			mSwapChain->ResizeBuffers(
				2,
				static_cast<UINT>(windowSize.Width),
				static_cast<UINT>(windowSize.Height),
				DXGI_FORMAT_B8G8R8A8_UNORM,
				0
			);
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
		critical_section::scoped_lock lock{ mControllersLock };
		if (mLeftPlayerController == nullptr) {
			mLeftPlayerController = gamepad;
			mLeftPlayerName = gamepad->User->NonRoamableId->Data();
		} else if (mRightPlayerController == nullptr) {
			mRightPlayerController = gamepad;
			mRightPlayerName = gamepad->User->NonRoamableId->Data();
		}
	}

	void GamepadRemoved(Object^ o, Gamepad^ gamepad)
	{
		critical_section::scoped_lock lock{ mControllersLock };
		if (mLeftPlayerController == gamepad) {
			mLeftPlayerController = nullptr;
			mLeftPlayerName = LEFT_PLAYER_NAME_PLACEHOLDER;
			Pause();
		} else if (mRightPlayerController == gamepad) {
			mRightPlayerController = nullptr;
			mRightPlayerName = RIGHT_PLAYER_NAME_PLACEHOLDER;
			Pause();
		}
	}

	void Pause()
	{
		// TODO
	}

	void CheckInput()
	{
		// TODO
	}

	void Update(int dt)
	{
		mBufferIdx = (mBufferIdx + 1) % 2;
		auto prevBufferIdx = (mBufferIdx + 1) % 2;

		// apply movement for the left paddle.
		mLeftPaddleRects[mBufferIdx].top = mLeftPaddleRects[prevBufferIdx].top + mLeftPaddleVelocity;
		mLeftPaddleRects[mBufferIdx].bottom = mLeftPaddleRects[prevBufferIdx].bottom + mLeftPaddleVelocity;

		// apply movement for the right paddle.
		mRightPaddleRects[mBufferIdx].top = mRightPaddleRects[prevBufferIdx].top + mRightPaddleVelocity;
		mRightPaddleRects[mBufferIdx].bottom = mRightPaddleRects[prevBufferIdx].bottom + mRightPaddleVelocity;

		// apply movement for the ball.
		auto ballMovement = XMVectorScale(mBallDirection, mBallVelocity);
		mBallRects[mBufferIdx].top = mBallRects[prevBufferIdx].top + ballMovement.m128_f32[1];
		mBallRects[mBufferIdx].bottom = mBallRects[prevBufferIdx].bottom + ballMovement.m128_f32[1];
		mBallRects[mBufferIdx].right = mBallRects[prevBufferIdx].right + ballMovement.m128_f32[0];
		mBallRects[mBufferIdx].left = mBallRects[prevBufferIdx].left + ballMovement.m128_f32[0];

		// check that the left paddle stays between the top and bottom wall.
		if (Collides(mLeftPaddleRects[mBufferIdx], mBottomWallRect)) {
			auto paddleHeight = mLeftPaddleRects[mBufferIdx].bottom - mLeftPaddleRects[mBufferIdx].top;
			mLeftPaddleRects[mBufferIdx].bottom = mBottomWallRect.top;
			mLeftPaddleRects[mBufferIdx].top = mBottomWallRect.top - paddleHeight;
		} else if (Collides(mLeftPaddleRects[mBufferIdx], mTopWallRect)) {
			auto paddleHeight = mLeftPaddleRects[mBufferIdx].bottom - mLeftPaddleRects[mBufferIdx].top;
			mLeftPaddleRects[mBufferIdx].bottom = mTopWallRect.bottom + paddleHeight;
			mLeftPaddleRects[mBufferIdx].top = mTopWallRect.bottom;
		}

		// check that the right paddle stays between the top and bottom wall.
		if (Collides(mRightPaddleRects[mBufferIdx], mBottomWallRect)) {
			auto paddleHeight = mRightPaddleRects[mBufferIdx].bottom - mRightPaddleRects[mBufferIdx].top;
			mRightPaddleRects[mBufferIdx].bottom = mBottomWallRect.top;
			mRightPaddleRects[mBufferIdx].top = mBottomWallRect.top - paddleHeight;
		} else if (Collides(mRightPaddleRects[mBufferIdx], mTopWallRect)) {
			auto paddleHeight = mRightPaddleRects[mBufferIdx].bottom - mRightPaddleRects[mBufferIdx].top;
			mRightPaddleRects[mBufferIdx].bottom = mTopWallRect.bottom + paddleHeight;
			mRightPaddleRects[mBufferIdx].top = mTopWallRect.bottom;
		}

		// check whether the ball has reached a goal.
		if (Contains(mLeftGoalRect, mBallRects[mBufferIdx])) {
			// TODO reset ball state
			// TODO randomize ball direction
			// TODO reset paddle states
			mRightPoints++;
		} else if (Contains(mRightGoalRect, mBallRects[mBufferIdx])) {
			// TODO reset ball state
			// TODO randomize ball direction
			// TODO reset paddle states
			mLeftPoints++;
		}

		if (Collides(mBallRects[mBufferIdx], mTopWallRect)) {
			// TODO handle
			mBallDirection.m128_f32[1] = -mBallDirection.m128_f32[1];
		} else if (Collides(mBallRects[mBufferIdx], mBottomWallRect)) {
			// TODO handle
			mBallDirection.m128_f32[1] = -mBallDirection.m128_f32[1];
		}

		if (Collides(mBallRects[mBufferIdx], mRightPaddleRects[mBufferIdx])) {
			// TODO handle
		} else if (Collides(mBallRects[mBufferIdx], mLeftPaddleRects[mBufferIdx])) {
			// TODO handle
		}
	}

	void Render(float alpha)
	{
		m2dCtx->BeginDraw();
		m2dCtx->Clear(D2D1::ColorF(D2D1::ColorF::Black));

		// static objects

		for (auto i = 0; i < CENTERLINE_DOTS; i++) {
			m2dCtx->FillRectangle(mCenterlineRects[i], mWhiteBrush.Get());
		}
		m2dCtx->FillRectangle(mTopWallRect, mWhiteBrush.Get());
		m2dCtx->FillRectangle(mBottomWallRect, mWhiteBrush.Get());
		m2dCtx->DrawText(
			std::to_wstring(mLeftPoints).c_str(),
			1,
			mPointsTextFormat.Get(),
			mLeftPointsRect,
			mWhiteBrush.Get(),
			nullptr
		);
		m2dCtx->DrawText(
			std::to_wstring(mRightPoints).c_str(),
			1,
			mPointsTextFormat.Get(),
			mRightPointsRect,
			mWhiteBrush.Get(),
			nullptr
		);
		m2dCtx->DrawText(
			mLeftPlayerName.c_str(),
			static_cast<UINT32>(mLeftPlayerName.size()),
			mLeftPlayerNameTextFormat.Get(),
			mLeftPlayerNameRect,
			mBlackBrush.Get(),
			nullptr
		);
		m2dCtx->DrawText(
			mRightPlayerName.c_str(),
			static_cast<UINT32>(mRightPlayerName.size()),
			mRightPlayerNameTextFormat.Get(),
			mRightPlayerNameRect,
			mBlackBrush.Get(),
			nullptr
		);

		m2dCtx->FillRectangle(mRightGoalRect, mWhiteBrush.Get());
		// dynamic objects

		auto prevBufferIdx = mBufferIdx == 0 ? 1 : 0;

		m2dCtx->FillRectangle(Interpolate(mLeftPaddleRects[prevBufferIdx], mLeftPaddleRects[mBufferIdx], alpha), mWhiteBrush.Get());
		m2dCtx->FillRectangle(Interpolate(mRightPaddleRects[prevBufferIdx], mRightPaddleRects[mBufferIdx], alpha), mWhiteBrush.Get());
		m2dCtx->FillRectangle(Interpolate(mBallRects[prevBufferIdx], mBallRects[mBufferIdx], alpha), mWhiteBrush.Get());
		
		ThrowIfFailed(m2dCtx->EndDraw());
		ThrowIfFailed(mSwapChain->Present(1, 0));
	}
private:
	// The flag used to stop execution of the application's main loop when the main window is closed.
	bool mWindowClosed;
	// The flag used to indicate whether the main window is visible and game should be rendered.
	bool mWindowVisible;

	float mPaddleVelocity = 0.f;
	float mBallVelocity = 0.f;

	uint8_t mLeftPoints;
	uint8_t mRightPoints;

	std::wstring mLeftPlayerName;
	std::wstring mRightPlayerName;

	Gamepad^			mLeftPlayerController;
	Gamepad^			mRightPlayerController;
	critical_section	mControllersLock;

	float mLeftPaddleVelocity = 0.f;
	float mRightPaddleVelocity = 0.f;

	XMVECTOR mBallDirection;

	ComPtr<ID3D11Device>		m3dDevice;
	ComPtr<ID3D11DeviceContext>	m3dCtx;
	ComPtr<ID2D1Factory6>		m2dFactory;
	ComPtr<ID2D1Device5>		m2dDevice;
	ComPtr<ID2D1DeviceContext5>	m2dCtx;
	ComPtr<IDXGISwapChain1>		mSwapChain;
	ComPtr<IDWriteFactory>		mWritefactory;

	ComPtr<ID2D1SolidColorBrush> mWhiteBrush;
	ComPtr<ID2D1SolidColorBrush> mBlackBrush;

	ComPtr<IDWriteTextFormat> mPointsTextFormat;
	ComPtr<IDWriteTextFormat> mLeftPlayerNameTextFormat;
	ComPtr<IDWriteTextFormat> mRightPlayerNameTextFormat;

	D2D1_RECT_F mCenterlineRects[CENTERLINE_DOTS];
	D2D1_RECT_F mTopWallRect;
	D2D1_RECT_F mBottomWallRect;
	D2D1_RECT_F mLeftPointsRect;
	D2D1_RECT_F mRightPointsRect;
	D2D1_RECT_F mLeftPlayerNameRect;
	D2D1_RECT_F mRightPlayerNameRect;
	D2D1_RECT_F mLeftPaddleRects[2];
	D2D1_RECT_F mRightPaddleRects[2];
	D2D1_RECT_F mBallRects[2];
	D2D1_RECT_F mLeftGoalRect;
	D2D1_RECT_F mRightGoalRect;

	int mBufferIdx = 0;
};

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	CoreApplication::Run(ref new Pong());
	return 0;
}