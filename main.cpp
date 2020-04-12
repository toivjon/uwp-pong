#include <concrt.h>
#include <cwchar>
#include <d2d1_3.h>
#include <d3d11.h>
#include <dwrite.h>
#include <dxgi1_6.h>
#include <ppltasks.h>
#include <string>
#include <wrl.h>

// DirectXTK
#include <Audio.h>		
#include <SimpleMath.h>

#include "geometry.h"
#include "util.h"

#include <cassert>

using namespace pong;

using namespace concurrency;
using namespace DirectX;
using namespace DirectX::SimpleMath;
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
constexpr auto LEFT_PLAYER_NAME_PLACEHOLDER = L"player-1";
// The placeholder name for the right player name.
constexpr auto RIGHT_PLAYER_NAME_PLACEHOLDER = L"player-2";
// The constant aspect ratio for the game content.
constexpr auto ASPECT_RATIO = (800.f / 600.f);
// The epsilon constant for floating point arithmetic.
constexpr auto EPSILON = 0.001f;
// The amount of milliseconds to wait before each ball launch.
constexpr auto COUNTDOWN_MS = 1000;
// The amount to push object from obstacle when colliding.
constexpr auto NUDGE = 0.001f;
// The scalar used to increase ball velocity on each hit with a paddle.
constexpr auto BALL_SPEEDUP_SCALAR = 1.2f;
// The tolerance used to detect whether the thumbstick is not pressed.
constexpr auto GAMEPAD_DEADZONE = 0.1f;
// The amount of power to put into gamepad vibration when ball hits paddle (0.f - 1.f).
constexpr auto GAMEPAD_FEEDBACK_STRENGTH = 0.75f;
// The duration of the gamepad vibration when ball hits paddle.
constexpr auto GAMEPAD_FEEDBACK_DURATION_MS = 200;
// The amount of points player needs to collect to win the game.
constexpr auto POINT_TARGET = 10;
// The id and ordinal of the left player.
constexpr uint8_t PLAYER_LEFT  = 0;
// The id and ordinal of the right player.
constexpr uint8_t PLAYER_RIGHT = 1;

// =================
// === Utilities ===
// =================

// A utility function to throw an exception if HRESULT was failed.
inline void ThrowIfFailed(HRESULT hr) {
	if (FAILED(hr)) {
		throw Platform::Exception::CreateException(hr);
	}
}

inline float SweptAABB(const D2D1_RECT_F& a, const D2D1_RECT_F& b, float vx, float vy, float& nx, float& ny) {
	float xInvEntry, xInvExit, xEntry, xExit;
	if (vx > 0.f) {
		// if a-rect is moving right...
		// -- calculate distance between closest pointt (entry)
		// -- calculate distance between farthest points (exit)
		xInvEntry = b.left - a.right;
		xInvExit = b.right - a.left;
	}
	else {
		// if a-rect is moving left...
		// -- calculate distance between closest points (entry)
		// -- calculate distance between farthest points (exit)
		xInvEntry = b.right - a.left;
		xInvExit = b.left - a.right;
	}

	float yInvEntry, yInvExit, yEntry, yExit;
	if (vy > 0.f) {
		yInvEntry = b.top - a.bottom;
		yInvExit = b.bottom - a.top;
	}
	else {
		yInvEntry = b.bottom - a.top;
		yInvExit = b.top - a.bottom;
	}

	xEntry = xInvEntry / vx;
	xExit = xInvExit / vx;
	yEntry = yInvEntry / vy;
	yExit = yInvExit / vy;

	auto entryTime = max(xEntry, yEntry);
	auto exitTime = min(xExit, yExit);

	if (entryTime > exitTime || xEntry < 0.f && yEntry < 0.f || xEntry > 1.f || yEntry > 1.f) {
		nx = 0.f;
		ny = 0.f;
		return 1.f;
	}
	else {
		if (xEntry > yEntry) {
			if (xInvEntry < 0.f) {
				nx = 1.f;
				ny = 0.f;
			}
			else {
				nx = -1.f;
				ny = 0.f;
			}
		}
		else {
			if (yInvEntry < 0.f) {
				nx = 0.f;
				ny = 1.f;
			}
			else {
				nx = 0.f;
				ny = -1.f;
			}
		}
	}
	return entryTime;
}


inline bool Intersect(const D2D1_RECT_F& a, const D2D1_RECT_F& b, const XMVECTOR& av, const XMVECTOR& bv, float& tmin, XMVECTOR& n) {
	tmin = -FLT_MAX;
	auto tmax = 1.f;
	auto v = (abs(bv.m128_f32[1]) > 0.f ? XMVectorSubtract(bv, av) : av);
	for (auto i = 0u; i < 2; i++) {
		auto amax = (i == 0 ? a.right : a.bottom);
		auto amin = (i == 0 ? a.left : a.top);
		auto bmax = (i == 0 ? b.right : b.bottom);
		auto bmin = (i == 0 ? b.left : b.top);
		auto t1 = (amax - bmin) / v.m128_f32[i]; // the time to a to collide with b (on direct collision, this is zero or negative)
		auto t2 = (amin - bmax) / v.m128_f32[i]; // the time to a to exit from b
		if (v.m128_f32[i] <= 0.f) {
			if (bmax < amin) return false; // no overlap and moving away from each other
			// if (amax <= bmin) {
				if (i == 0u) {
					n = XMVectorSet(-1.f, 0.f, 0.f, 0.f);
				} else if (t1 > tmin) {
					n = XMVectorSet(0.f, -1.f, 0.f, 0.f);
				}
				tmin = max(t1, tmin);
			// }
			if (bmax > amin) tmax = min(t2, tmax);
		} else if (v.m128_f32[i] > 0.f) {
			if (bmin > amax) return false; // no overlap and moving away from each other
			// if (bmax < amin) {
				if (i == 0u) {
					n = XMVectorSet(1.f, 0.f, 0.f, 0.f);
				} else if (t1 > tmin) {
					n = XMVectorSet(0.f, 1.f, 0.f, 0.f);
				}
				tmin = max(t1, tmin);
			// }
			if (amax > bmin) tmax = min(t2, tmax);
		}
		if (tmin > tmax) return false;
	}
	return true;
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
		InitializeAudio();
		InitializeGraphics();
		InitializeGame();
	}

	void InitializeAudio()
	{
		assert(mAudioEngine == nullptr);
		assert(mBeepSound == nullptr);

		mAudioEngine = std::make_unique<AudioEngine>();
		mBeepSound = std::make_unique<SoundEffect>(mAudioEngine.get(), L"Assets/beep.wav");
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
		RandomizeBallDirection();
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
		case VirtualKey::Enter:
			if (mLeftPoints >= POINT_TARGET || mRightPoints >= POINT_TARGET) {
				ResetGame();
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
		auto millisAccumulator = 0ll;
		auto oldMillis = util::GetCurrentMilliseconds();
		while (!mWindowClosed) {
			auto window = CoreWindow::GetForCurrentThread();
			if (mWindowVisible) {
				window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
				CheckInput();
				mAudioEngine->Update();

				// calculate the time usable for the current frame.
				auto newMillis = util::GetCurrentMilliseconds();
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
		auto window = CoreWindow::GetForCurrentThread();
		window->Activate();
		auto windowBounds = window->Bounds;
		auto width = windowBounds.Width;
		auto height = windowBounds.Height;
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
		auto oldWidth = mWindowWidth;
		auto oldHeight = mWindowHeight;
		auto oldCellSize = mCellSize;

		// get and calculate some general dimensions for the game.
		auto window = CoreWindow::GetForCurrentThread();
		mWindowWidth = window->Bounds.Width;
		mWindowHeight = window->Bounds.Height;

		// calculate aspect ratio correction spacing values.
		auto currentAspectRatio = (mWindowWidth / mWindowHeight);
		mWindowWidthSpacing = 0.f, mWindowHeightSpacing = 0.f;
		if (currentAspectRatio > ASPECT_RATIO) {
			auto widthEdge = mWindowHeight * ASPECT_RATIO;
			mWindowWidthSpacing = mWindowWidth - widthEdge;
		} else if (currentAspectRatio < ASPECT_RATIO) {
			auto heightEdge = mWindowWidth / ASPECT_RATIO;
			mWindowHeightSpacing = mWindowHeight - heightEdge;
		}

		// precalculate the game object cell size.
		mCellSize = (mWindowHeight - mWindowHeightSpacing) / 30;

		ResizeGameObjects(oldWidth, oldHeight, oldCellSize, window);
		ResizeSwapchain(window);
	}

	void ResizeGameObjects(float oldWidth, float oldHeight, float oldCellSize, CoreWindow^ window)
	{
		// calculate physical coefficients.
		mPaddleVelocity = (mWindowHeight - mWindowHeightSpacing) / 30;
		if (oldCellSize > EPSILON) {
			auto scalar = mBallVelocity / oldCellSize;
			mBallVelocity = mCellSize * scalar;
		} else {
			mBallVelocity = mPaddleVelocity / 4;
		}

		// correct left paddle velocity if currently applied.
		if (mLeftPaddleVelocity > EPSILON || mLeftPaddleVelocity < -EPSILON) {
			mLeftPaddleVelocity = mPaddleVelocity;
		}

		// correct right paddle velocity if currently applied.
		if (mRightPaddleVelocity > EPSILON || mRightPaddleVelocity < -EPSILON) {
			mRightPaddleVelocity = mPaddleVelocity;
		}

		// calculate view center points.
		auto horizontalCenter = mWindowWidth / 2;
		auto verticalCenter = mWindowHeight / 2;

		mTopWallRect.top = mWindowHeightSpacing / 2;
		mTopWallRect.bottom = mTopWallRect.top + mCellSize;
		mTopWallRect.left = mWindowWidthSpacing / 2;
		mTopWallRect.right = mWindowWidth - mWindowWidthSpacing / 2;

		mBottomWallRect.top = mWindowHeight - mWindowHeightSpacing / 2 - mCellSize;
		mBottomWallRect.bottom = mBottomWallRect.top + mCellSize;
		mBottomWallRect.left = mWindowWidthSpacing / 2;
		mBottomWallRect.right = mWindowWidth - mWindowWidthSpacing / 2;

		ThrowIfFailed(mWritefactory->CreateTextFormat(
			L"Calibri",
			nullptr,
			DWRITE_FONT_WEIGHT_REGULAR,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			mCellSize * 6,
			L"en-us",
			&mPointsTextFormat
		));
		ThrowIfFailed(mPointsTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
		ThrowIfFailed(mPointsTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));

		mLeftPointsRect.top = mWindowHeightSpacing / 2 + mCellSize * 2;
		mLeftPointsRect.bottom = mLeftPointsRect.top + mCellSize * 6;
		mLeftPointsRect.left = horizontalCenter - mCellSize * 5;
		mLeftPointsRect.right = mLeftPointsRect.left + mCellSize;

		mRightPointsRect.top = mWindowHeightSpacing / 2 + mCellSize * 2;
		mRightPointsRect.bottom = mRightPointsRect.top + mCellSize * 6;
		mRightPointsRect.left = horizontalCenter + mCellSize * 4;
		mRightPointsRect.right = mRightPointsRect.left + mCellSize;

		ThrowIfFailed(mWritefactory->CreateTextFormat(
			L"Calibri",
			nullptr,
			DWRITE_FONT_WEIGHT_REGULAR,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			mCellSize * 0.75f,
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
			mCellSize * 0.75f,
			L"en-us",
			&mRightPlayerNameTextFormat
		));
		ThrowIfFailed(mRightPlayerNameTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING));
		ThrowIfFailed(mRightPlayerNameTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));

		mLeftPlayerNameRect.top = mWindowHeightSpacing / 2;
		mLeftPlayerNameRect.bottom = mLeftPlayerNameRect.top + mCellSize;
		mLeftPlayerNameRect.left = mWindowWidthSpacing / 2 + mCellSize * .5f;
		mLeftPlayerNameRect.right = horizontalCenter - mCellSize * 3;

		mRightPlayerNameRect.top = mWindowHeightSpacing / 2;
		mRightPlayerNameRect.bottom = mRightPlayerNameRect.top + mCellSize;
		mRightPlayerNameRect.left = horizontalCenter + mCellSize * 3;
		mRightPlayerNameRect.right = mWindowWidth - mWindowWidthSpacing / 2 - mCellSize * .5f;

		for (auto i = 0; i < CENTERLINE_DOTS; i++) {
			mCenterlineRects[i].top = mWindowHeightSpacing / 2 + (i*2 + 0.5f) * mCellSize;
			mCenterlineRects[i].bottom = mCenterlineRects[i].top + mCellSize;
			mCenterlineRects[i].left = horizontalCenter - (mCellSize / 2);
			mCenterlineRects[i].right = mCenterlineRects[i].left + mCellSize;
		}

		mLeftGoalRect.top = 0;
		mLeftGoalRect.bottom = mWindowHeight;
		mLeftGoalRect.left = -D3D10_FLOAT32_MAX;
		mLeftGoalRect.right = mWindowWidthSpacing / 2;

		mRightGoalRect.top = 0;
		mRightGoalRect.bottom = mWindowHeight;
		mRightGoalRect.left = mWindowWidth - mWindowWidthSpacing / 2;
		mRightGoalRect.right = D3D10_FLOAT32_MAX;

		auto oldHalfWidth = oldWidth / 2.f;
		auto oldHalfHeight = oldHeight / 2.f;

		for (auto i = 0; i < 2; i++) {
			auto oldLeftRelMovement = abs(oldCellSize) <= EPSILON ? 0.f : (mLeftPaddleRects[i].top - (oldHalfHeight - (2.5f * oldCellSize))) / oldCellSize;
			auto oldRightRelMovement = abs(oldCellSize) <= EPSILON ? 0.f : (mRightPaddleRects[i].top - (oldHalfHeight - (2.5f * oldCellSize))) / oldCellSize;
			auto ballRelMovementY = abs(oldCellSize) <= EPSILON ? 0.f : (mBallRects[i].top - (oldHalfHeight - (.5f * oldCellSize))) / oldCellSize;
			auto ballRelMovementX = abs(oldCellSize) <= EPSILON ? 0.f : (mBallRects[i].left - (oldHalfWidth- (.5f * oldCellSize))) / oldCellSize;

			mLeftPaddleRects[i].top = verticalCenter - (2.5f * mCellSize) + mCellSize * oldLeftRelMovement;
			mLeftPaddleRects[i].bottom = mLeftPaddleRects[i].top + 5 * mCellSize;
			mLeftPaddleRects[i].left = mWindowWidthSpacing / 2 + mCellSize;
			mLeftPaddleRects[i].right = mLeftPaddleRects[i].left + mCellSize;

			mRightPaddleRects[i].top = verticalCenter - (2.5f * mCellSize) + mCellSize * oldRightRelMovement;
			mRightPaddleRects[i].bottom = mRightPaddleRects[i].top + 5 * mCellSize;
			mRightPaddleRects[i].left = mWindowWidth - (2 * mCellSize + mWindowWidthSpacing / 2);
			mRightPaddleRects[i].right = mRightPaddleRects[i].left + mCellSize;

			mBallRects[i].top = verticalCenter - (.5f * mCellSize) + mCellSize * ballRelMovementY;
			mBallRects[i].bottom = mBallRects[i].top + mCellSize; 
			mBallRects[i].left = horizontalCenter - (.5f * mCellSize) + mCellSize * ballRelMovementX;
			mBallRects[i].right = mBallRects[i].left + mCellSize;
		}

		// build game over specific objects
		ThrowIfFailed(mWritefactory->CreateTextFormat(
			L"Calibri",
			nullptr,
			DWRITE_FONT_WEIGHT_REGULAR,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			mCellSize * 3,
			L"en-us",
			&mGameOverBigTextFormat
			));
		ThrowIfFailed(mGameOverBigTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
		ThrowIfFailed(mWritefactory->CreateTextFormat(
			L"Calibri",
			nullptr,
			DWRITE_FONT_WEIGHT_REGULAR,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			mCellSize * 0.75f,
			L"en-us",
			&mGameOverSmallTextFormat
			));
		ThrowIfFailed(mGameOverSmallTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
		mGameOverRect = {
			horizontalCenter - 9 * mCellSize,
			verticalCenter   - 3 * mCellSize,
			horizontalCenter + 9 * mCellSize,
			verticalCenter   + 3 * mCellSize
		};
		mGameOverBigTextRect = {
			horizontalCenter - 9 * mCellSize,
			verticalCenter   - 3 * mCellSize,
			horizontalCenter + 9 * mCellSize,
			verticalCenter   + 3 * mCellSize
		};
		mGameOverSmallTextRect = {
			horizontalCenter - 10 * mCellSize,
			verticalCenter + mCellSize,
			horizontalCenter + 10 * mCellSize,
			verticalCenter   + 2 * mCellSize
		};
	}

	void ResetMovingObjects()
	{
		// precalculate the half window dimensions.
		auto halfWidth = mWindowWidth / 2;
		auto halfHeight = mWindowHeight / 2;

		// reset both buffers for each moving object.
		for (auto i = 0; i < 2; i++) {
			mLeftPaddleRects[i].top = halfHeight - (2.5f * mCellSize);
			mLeftPaddleRects[i].bottom = mLeftPaddleRects[i].top + 5 * mCellSize;
			mLeftPaddleRects[i].left = mWindowWidthSpacing / 2 + mCellSize;
			mLeftPaddleRects[i].right = mLeftPaddleRects[i].left + mCellSize;

			mRightPaddleRects[i].top = halfHeight - (2.5f * mCellSize);
			mRightPaddleRects[i].bottom = mRightPaddleRects[i].top + 5 * mCellSize;
			mRightPaddleRects[i].left = mWindowWidth - (2 * mCellSize + mWindowWidthSpacing / 2);
			mRightPaddleRects[i].right = mRightPaddleRects[i].left + mCellSize;

			mBallRects[i].top = halfHeight - (.5f * mCellSize);
			mBallRects[i].bottom = mBallRects[i].top + mCellSize;
			mBallRects[i].left = halfWidth - (.5f * mCellSize);
			mBallRects[i].right = mBallRects[i].left + mCellSize;
		}

		// reset ball velocity.
		mBallVelocity = (mWindowHeight - mWindowHeightSpacing) / 30 / 4;
	}

	void RandomizeBallDirection()
	{
		auto x = -1.f + (2.f * util::GetRandomIntBetween(0, 1));
		auto y = -1.f + (2.f * util::GetRandomIntBetween(0, 1));
		mBallDirection = Vector2(x, y);
		mBallDirection.Normalize();
	}

	void ResizeSwapchain(CoreWindow^ window)
	{
		DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();
		auto dpi = currentDisplayInformation->LogicalDpi;

		auto windowSize = Size(window->Bounds.Width, window->Bounds.Height);
		windowSize.Width = util::ConvertDipsToPixels(windowSize.Width, dpi);
		windowSize.Height = util::ConvertDipsToPixels(windowSize.Height, dpi);

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
			desc.Width = static_cast<UINT>(windowSize.Width);
			desc.Height = static_cast<UINT>(windowSize.Height);
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
		properties.dpiX = dpi;
		properties.dpiY = dpi;

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
		m2dCtx->SetDpi(dpi, dpi);
	}

	void GamepadAdded(Object^ o, Gamepad^ gamepad)
	{
		critical_section::scoped_lock lock{ mControllersLock };
		if (mLeftPlayerController == nullptr) {
			mLeftPlayerController = gamepad;
			auto user = gamepad->User;
			mLeftPlayerName = ((Platform::String^)create_task(user->GetPropertyAsync(KnownUserProperties::AccountName)).get())->Data();
		} else if (mRightPlayerController == nullptr) {
			mRightPlayerController = gamepad;
			auto user = gamepad->User;
			mRightPlayerName = ((Platform::String^)create_task(user->GetPropertyAsync(KnownUserProperties::AccountName)).get())->Data();
		}
	}

	void GamepadRemoved(Object^ o, Gamepad^ gamepad)
	{
		critical_section::scoped_lock lock{ mControllersLock };
		if (mLeftPlayerController == gamepad) {
			mLeftPlayerController = nullptr;
			mLeftPlayerName = LEFT_PLAYER_NAME_PLACEHOLDER;
			mLeftPaddleVelocity = 0.f;
			Pause();
		} else if (mRightPlayerController == gamepad) {
			mRightPlayerController = nullptr;
			mRightPlayerName = RIGHT_PLAYER_NAME_PLACEHOLDER;
			mRightPaddleVelocity = 0.f;
			Pause();
		}
	}

	void Pause()
	{
		// TODO
	}

	void CheckInput()
	{
		critical_section::scoped_lock lock{ mControllersLock };
		if (mLeftPlayerController != nullptr) {
			auto reading = mLeftPlayerController->GetCurrentReading();
			mLeftPaddleVelocity = abs(reading.LeftThumbstickY) < GAMEPAD_DEADZONE ? 0.f : static_cast<float>(reading.LeftThumbstickY) * -mPaddleVelocity;
			if (mLeftPoints >= POINT_TARGET || mRightPoints >= POINT_TARGET) {
				if (GamepadButtons::X == (reading.Buttons & GamepadButtons::X)) {
					ResetGame();
				}
			}
		}
		if (mRightPlayerController != nullptr) {
			auto reading = mRightPlayerController->GetCurrentReading();
			mRightPaddleVelocity = abs(reading.LeftThumbstickY) < GAMEPAD_DEADZONE ? 0.f : static_cast<float>(reading.LeftThumbstickY) * -mPaddleVelocity;
			if (mLeftPoints >= POINT_TARGET || mRightPoints >= POINT_TARGET) {
				if (GamepadButtons::X == (reading.Buttons & GamepadButtons::X)) {
					ResetGame();
				}
			}
		}
	}

	void ResetGame()
	{
		mLeftPoints = 0;
		mRightPoints = 0;
		mCountdown = COUNTDOWN_MS;
		ResetMovingObjects();
		RandomizeBallDirection();
	}

	void Update(int dt)
	{
		// don't update anything while the game is over.
		if (mLeftPoints >= POINT_TARGET || mRightPoints >= POINT_TARGET) {
			return;
		}

		// don't update anything while there's still countdown millis left.
		mCountdown = max(0, mCountdown - dt);
		if (mCountdown > 0) {
			return;
		}

		mBufferIdx = (mBufferIdx + 1) % 2;
		auto prevBufferIdx = (mBufferIdx + 1) % 2;

		// apply movement to paddles.
		auto leftPaddleMovement = XMVectorSet(0.f, mLeftPaddleVelocity, 0.f, 0.f);
		auto rightPaddleMovement = XMVectorSet(0.f, mRightPaddleVelocity, 0.f, 0.f);
		auto leftPaddlePosition = mLeftPaddleRects[prevBufferIdx];
		auto rightPaddlePosition = mRightPaddleRects[prevBufferIdx];
		leftPaddlePosition.Move(leftPaddleMovement.m128_f32[0], leftPaddleMovement.m128_f32[1]);
		rightPaddlePosition.Move(rightPaddleMovement.m128_f32[0], rightPaddleMovement.m128_f32[1]);
		mLeftPaddleRects[mBufferIdx] = leftPaddlePosition;
		mRightPaddleRects[mBufferIdx] = rightPaddlePosition;

		// check that the left paddle stays between the top and bottom wall.
		if (mLeftPaddleRects[mBufferIdx].Collides(mBottomWallRect)) {
			auto paddleHeight = mLeftPaddleRects[mBufferIdx].bottom - mLeftPaddleRects[mBufferIdx].top;
			mLeftPaddleRects[mBufferIdx].bottom = mBottomWallRect.top;
			mLeftPaddleRects[mBufferIdx].top = mBottomWallRect.top - paddleHeight;
		} else if (mLeftPaddleRects[mBufferIdx].Collides(mTopWallRect)) {
			auto paddleHeight = mLeftPaddleRects[mBufferIdx].bottom - mLeftPaddleRects[mBufferIdx].top;
			mLeftPaddleRects[mBufferIdx].bottom = mTopWallRect.bottom + paddleHeight;
			mLeftPaddleRects[mBufferIdx].top = mTopWallRect.bottom;
		}

		// check that the right paddle stays between the top and bottom wall.
		if (mRightPaddleRects[mBufferIdx].Collides(mBottomWallRect)) {
			auto paddleHeight = mRightPaddleRects[mBufferIdx].bottom - mRightPaddleRects[mBufferIdx].top;
			mRightPaddleRects[mBufferIdx].bottom = mBottomWallRect.top;
			mRightPaddleRects[mBufferIdx].top = mBottomWallRect.top - paddleHeight;
		} else if (mRightPaddleRects[mBufferIdx].Collides(mTopWallRect)) {
			auto paddleHeight = mRightPaddleRects[mBufferIdx].bottom - mRightPaddleRects[mBufferIdx].top;
			mRightPaddleRects[mBufferIdx].bottom = mTopWallRect.bottom + paddleHeight;
			mRightPaddleRects[mBufferIdx].top = mTopWallRect.bottom;
		}

		/*
		// TODO temporary test solution
		mBallDirection = XMVector2Normalize(XMVectorSet(.5f, .5f, 0.f, 0.f));
		auto tBall = 1.f;
		auto ballPosition = mBallRects[prevBufferIdx];
		auto movement = XMVectorScale(mBallDirection, mBallVelocity);
		movement = XMVectorScale(movement, tBall);
		auto hitTime = 0.f;
		auto hitNormal = XMVectorSet(0.f, 0.f, 0.f, 0.f);
		if (mBallDirection.m128_f32[1] > 0.f && Intersect(ballPosition, mBottomWallRect, movement, XMVECTOR(), hitTime, hitNormal)) {
			mBallDirection = XMVectorSet(0.f, 0.f, 0.f, 0.f);
			return;
		}
		ballPosition = MoveAABB(ballPosition, movement);
		mBallRects[mBufferIdx] = ballPosition;
		*/
		auto tBall = static_cast<float>(dt);
		auto ballPosition = mBallRects[prevBufferIdx];
		while (tBall > 0.f) {
			// create a movement vector scaled with the time left for the ball.
			auto movement = XMVectorScale(mBallDirection, mBallVelocity * tBall);
			// movement = XMVectorScale(movement, tBall);

			auto hitTime = 0.f;
			auto hitNormal = XMVectorSet(0.f, 0.f, 0.f, 0.f);
			if (mBallDirection.y < 0.f && Intersect(ballPosition, mTopWallRect, movement, XMVECTOR(), hitTime, hitNormal)) {
				// move the ball straight to the hit point.
				// TODO old... movement = XMVectorScale(movement, hitTime);
				// movement = XMVectorScale(mBallDirection, mBallVelocity * tBall * hitTime);
				movement = XMVectorScale(movement, hitTime);
				ballPosition.Move(movement.m128_f32[0], movement.m128_f32[1]);

				// inverse balls vertical movement direction.
				mBallDirection.y = -mBallDirection.y;

				mBallDirection = XMVECTOR();
				tBall = 0.f;
				// ballPosition = mBallRects[prevBufferIdx];

				/*
				// apply a small nudge to make the ball to leave collision area.
				movement = XMVectorScale(mBallDirection, mBallVelocity);
				movement = XMVectorScale(movement, NUDGE);
				ballPosition = MoveAABB(ballPosition, movement);

				// decrease the amount of usable time for ball movement.
				tBall -= hitTime;
				mBeepSound.Play();
				*/
			} else if (mBallDirection.y > 0.f && Intersect(ballPosition, mBottomWallRect, movement, XMVECTOR(), hitTime, hitNormal)) {
				// move the ball straight to the hit point.
				movement = XMVectorScale(movement, hitTime);
				ballPosition.Move(movement.m128_f32[0], movement.m128_f32[1]);

				// inverse balls vertical movement direction.
				mBallDirection.y = -mBallDirection.y;

				// apply a small nudge to make the ball to leave collision area.
				movement = XMVectorScale(mBallDirection, mBallVelocity);
				movement = XMVectorScale(movement, NUDGE);
				ballPosition.Move(movement.m128_f32[0], movement.m128_f32[1]);

				// decrease the amount of usable time for ball movement.
				tBall -= hitTime;
				mBeepSound->Play();
			} else if (mBallDirection.y < 0.f && Intersect(ballPosition, mLeftPaddleRects[prevBufferIdx], movement, leftPaddleMovement, hitTime, hitNormal)) {
				// move the ball straight to the hit point.
				movement = XMVectorScale(movement, hitTime);
				ballPosition.Move(movement.m128_f32[0], movement.m128_f32[1]);

				// inverse balls horizontal movement direction.
				mBallDirection.y = -mBallDirection.y;

				// let's increase ball velocity on each paddle hit.
				mBallVelocity *= BALL_SPEEDUP_SCALAR;

				// decrease the amount of usable time for ball movement.
				tBall -= hitTime;

				// vibrate left player controller.
				critical_section::scoped_lock lock{ mControllersLock };
				if (mLeftPlayerController != nullptr) {
					auto vibration = mLeftPlayerController->Vibration;
					vibration.LeftMotor = GAMEPAD_FEEDBACK_STRENGTH;
					vibration.RightMotor = GAMEPAD_FEEDBACK_STRENGTH;
					mLeftPlayerController->Vibration = vibration;
					auto controller = mLeftPlayerController;
					create_task([controller] {
						std::this_thread::sleep_for(std::chrono::milliseconds(GAMEPAD_FEEDBACK_DURATION_MS));
						auto vibration = controller->Vibration;
						vibration.LeftMotor = 0.f;
						vibration.RightMotor = 0.f;
						controller->Vibration = vibration;
						});
				}
				mBeepSound->Play();
			} else if (mBallDirection.y > 0.f && Intersect(ballPosition, mRightPaddleRects[prevBufferIdx], movement, rightPaddleMovement, hitTime, hitNormal)) {
				// move the ball straight to the hit point.
				movement = XMVectorScale(movement, hitTime);
				ballPosition.Move(movement.m128_f32[0], movement.m128_f32[1]);

				// inverse balls horizontal movement direction.
				mBallDirection.y = -mBallDirection.y;

				// let's increase ball velocity on each paddle hit.
				mBallVelocity *= BALL_SPEEDUP_SCALAR;

				// decrease the amount of usable time for ball movement.
				tBall -= hitTime;

				// vibrate right player controller.
				critical_section::scoped_lock lock{ mControllersLock };
				if (mRightPlayerController != nullptr) {
					auto vibration = mRightPlayerController->Vibration;
					vibration.LeftMotor = GAMEPAD_FEEDBACK_STRENGTH;
					vibration.RightMotor = GAMEPAD_FEEDBACK_STRENGTH;
					mRightPlayerController->Vibration = vibration;
					auto controller = mRightPlayerController;
					create_task([controller] {
						std::this_thread::sleep_for(std::chrono::milliseconds(GAMEPAD_FEEDBACK_DURATION_MS));
						auto vibration = controller->Vibration;
						vibration.LeftMotor = 0.f;
						vibration.RightMotor = 0.f;
						controller->Vibration = vibration;
						});
				}
				mBeepSound->Play();
			} else {
				// move the ball as far as possible.
				ballPosition.Move(movement.m128_f32[0], movement.m128_f32[1]);
				tBall = 0.f;
			}
		}
		mBallRects[mBufferIdx] = ballPosition;

		// check whether the ball has reached a goal.
		if (mLeftGoalRect.Contains(mBallRects[mBufferIdx])) {
			handleGoal(PLAYER_RIGHT);
		} else if (mRightGoalRect.Contains(mBallRects[mBufferIdx])) {
			handleGoal(PLAYER_LEFT);
		}
	}

	void handleGoal(uint8_t player) {
		if (player == PLAYER_LEFT) {
			mLeftPoints++;
		} else {
			mRightPoints++;
		}
		mCountdown = COUNTDOWN_MS;
		ResetMovingObjects();
		RandomizeBallDirection();
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

		m2dCtx->FillRectangle(geometry::Rectangle::Lerp(
			mLeftPaddleRects[prevBufferIdx],
			mLeftPaddleRects[mBufferIdx],
			alpha),
			mWhiteBrush.Get());
		m2dCtx->FillRectangle(geometry::Rectangle::Lerp(
			mRightPaddleRects[prevBufferIdx],
			mRightPaddleRects[mBufferIdx],
			alpha),
			mWhiteBrush.Get());
		m2dCtx->FillRectangle(geometry::Rectangle::Lerp(
			mBallRects[prevBufferIdx],
			mBallRects[mBufferIdx],
			alpha),
			mWhiteBrush.Get());

		// draw the game over box if the game is over
		if (mRightPoints >= POINT_TARGET || mLeftPoints >= POINT_TARGET) {
			m2dCtx->FillRectangle(mGameOverRect, mWhiteBrush.Get());
			m2dCtx->DrawText(
				L"GAME OVER",
				9,
				mGameOverBigTextFormat.Get(),
				mGameOverBigTextRect,
				mBlackBrush.Get(),
				nullptr
				);
			m2dCtx->DrawText(
				L"Press Gamepad X or Keyboard Enter To Continue",
				46,
				mGameOverSmallTextFormat.Get(),
				mGameOverSmallTextRect,
				mBlackBrush.Get(),
				nullptr
				);
		}

		ThrowIfFailed(m2dCtx->EndDraw());
		ThrowIfFailed(mSwapChain->Present(1, 0));
	}
private:
	// The flag used to stop execution of the application's main loop when the main window is closed.
	bool mWindowClosed;
	// The flag used to indicate whether the main window is visible and game should be rendered.
	bool mWindowVisible;
	// The width in pixels of the window attached for the game.
	float mWindowWidth;
	// The height in pixels of the window attached for the game.
	float mWindowHeight;
	// The horizontal spacing required to maintain the constant aspect ratio.
	float mWindowWidthSpacing;
	// The vertical spacing required to maintain the constant aspect ratio.
	float mWindowHeightSpacing;

	// The size of single game object cell used to form all game objects.
	float mCellSize;
	// The timer for a small countdown before each ball launch.
	int mCountdown = COUNTDOWN_MS;

	float mPaddleVelocity = 0.f;
	float mBallVelocity = 0.f;

	uint8_t mLeftPoints;
	uint8_t mRightPoints;

	std::wstring mLeftPlayerName = LEFT_PLAYER_NAME_PLACEHOLDER;
	std::wstring mRightPlayerName = RIGHT_PLAYER_NAME_PLACEHOLDER;

	Gamepad^			mLeftPlayerController;
	Gamepad^			mRightPlayerController;
	critical_section	mControllersLock;

	float mLeftPaddleVelocity = 0.f;
	float mRightPaddleVelocity = 0.f;

	Vector2 mBallDirection;

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

	geometry::Rectangle mCenterlineRects[CENTERLINE_DOTS];
	geometry::Rectangle mTopWallRect;
	geometry::Rectangle mBottomWallRect;
	geometry::Rectangle mLeftPointsRect;
	geometry::Rectangle mRightPointsRect;
	geometry::Rectangle mLeftPlayerNameRect;
	geometry::Rectangle mRightPlayerNameRect;
	geometry::Rectangle mLeftPaddleRects[2];
	geometry::Rectangle mRightPaddleRects[2];
	geometry::Rectangle mBallRects[2];
	geometry::Rectangle mLeftGoalRect;
	geometry::Rectangle mRightGoalRect;

	geometry::Rectangle mGameOverRect;
	geometry::Rectangle mGameOverBigTextRect;
	geometry::Rectangle mGameOverSmallTextRect;
	ComPtr<IDWriteTextFormat> mGameOverBigTextFormat;
	ComPtr<IDWriteTextFormat> mGameOverSmallTextFormat;

	int mBufferIdx = 0;

	std::unique_ptr<AudioEngine> mAudioEngine;
	std::unique_ptr<SoundEffect> mBeepSound;
};

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	CoreApplication::Run(ref new Pong());
	return 0;
}