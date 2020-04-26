// a list of couple TODOs....
// 1. [keyboard] When resuming from a pause or suspend, be sure to call Reset on the tracker object to clear the state history.

#include <concrt.h>
#include <cwchar>
#include <ppltasks.h>
#include <string>

#include "entity.h"
#include "geometry.h"
#include "graphics.h"
#include "player.h"
#include "util.h"

// DirectXTK
#include <Audio.h>
#include <Keyboard.h>
#include <SimpleMath.h>

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
constexpr uint8_t PLAYER_LEFT = 0;
// The id and ordinal of the right player.
constexpr uint8_t PLAYER_RIGHT = 1;
// The title shown in the game over dialog.
constexpr auto GAME_OVER_TITLE = L"GAME OVER";
// The description shown in the game over dialog.
constexpr auto GAME_OVER_DESCRIPTON = L"Press Gamepad X or Keyboard Enter To Continue";

// =================
// === Utilities ===
// =================

inline float SweptAABB(const D2D1_RECT_F& a, const D2D1_RECT_F& b, float vx, float vy, float& nx, float& ny) {
	float xInvEntry, xInvExit, xEntry, xExit;
	if (vx > 0.f) {
		// if a-rect is moving right...
		// -- calculate distance between closest pointt (entry)
		// -- calculate distance between farthest points (exit)
		xInvEntry = b.left - a.right;
		xInvExit = b.right - a.left;
	} else {
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
	} else {
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
	} else {
		if (xEntry > yEntry) {
			if (xInvEntry < 0.f) {
				nx = 1.f;
				ny = 0.f;
			} else {
				nx = -1.f;
				ny = 0.f;
			}
		} else {
			if (yInvEntry < 0.f) {
				nx = 0.f;
				ny = 1.f;
			} else {
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
		} else {
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
	virtual IFrameworkView^ CreateView() {
		return ref new Pong();
	}

	virtual void Initialize(CoreApplicationView^ view) {
		view->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &Pong::Activated);
		Gamepad::GamepadAdded += ref new EventHandler<Gamepad^>(this, &Pong::GamepadAdded);
		Gamepad::GamepadRemoved += ref new EventHandler<Gamepad^>(this, &Pong::GamepadRemoved);
		mAudioEngine = std::make_unique<AudioEngine>();
		mBeepSound = std::make_unique<SoundEffect>(mAudioEngine.get(), L"Assets/beep.wav");
		mGraphics = std::make_unique<graphics::Graphics>();
		RandomizeBallDirection();
		mKeyboard = std::make_unique<Keyboard>();

		mLeftPlayer = std::make_unique<Player>();
		mRightPlayer = std::make_unique<Player>();
	}

	virtual void SetWindow(CoreWindow^ window) {
		window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &Pong::WindowClosed);
		window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &Pong::WindowVisibilityChanged);
		window->SizeChanged += ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &Pong::WindowSizeChanged);
		mKeyboard->SetWindow(window);
		ResizeContent(window);
	}

	virtual void Load(Platform::String^) {
		// ... no operations required
	}

	virtual void Run() {
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
			} else {
				window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
			}
		}
	}

	virtual void Uninitialize() {
		// ... no operations required
	}

	void WindowClosed(CoreWindow^, CoreWindowEventArgs^) {
		mWindowClosed = true;
	}

	void Activated(CoreApplicationView^, IActivatedEventArgs^) {
		CoreWindow::GetForCurrentThread()->Activate();
	}

	void WindowVisibilityChanged(CoreWindow^, VisibilityChangedEventArgs^ args) {
		mWindowVisible = args->Visible;
	}

	void WindowSizeChanged(CoreWindow^ window, WindowSizeChangedEventArgs^) {
		ResizeContent(window);
	}

	void ResizeContent(CoreWindow^ window) {
		auto oldWidth = mWindowWidth;
		auto oldHeight = mWindowHeight;
		auto oldCellSize = mCellSize;

		// get and calculate some general dimensions for the game.
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
		mGraphics->SetCoreWindow(window);
	}

	void ResizeGameObjects(float oldWidth, float oldHeight, float oldCellSize, CoreWindow^ window) {
		// calculate physical coefficients.
		mPaddleVelocity = (mWindowHeight - mWindowHeightSpacing) / 30;
		if (oldCellSize > EPSILON) {
			auto scalar = mBallVelocity / oldCellSize;
			mBallVelocity = mCellSize * scalar;
		} else {
			mBallVelocity = mPaddleVelocity / 4;
		}

		// correct left paddle velocity if currently applied.
		if (mLeftPaddleVelocity.y > EPSILON || mLeftPaddleVelocity.y < -EPSILON) {
			mLeftPaddleVelocity.y = mPaddleVelocity;
		}

		// correct right paddle velocity if currently applied.
		if (mRightPaddleVelocity.y > EPSILON || mRightPaddleVelocity.y < -EPSILON) {
			mRightPaddleVelocity.y = mPaddleVelocity;
		}

		// calculate view center points.
		auto horizontalCenter = mWindowWidth / 2;
		auto verticalCenter = mWindowHeight / 2;

		mTopWall.SetPosition(mWindowWidthSpacing / 2, mWindowHeightSpacing / 2);
		mTopWall.SetSize(mWindowWidth - mWindowWidthSpacing, mCellSize);

		mBottomWall.SetPosition(mWindowWidthSpacing / 2, mWindowHeight - mWindowHeightSpacing / 2 - mCellSize);
		mBottomWall.SetSize(mWindowWidth - mWindowWidthSpacing, mCellSize);

		mLeftPointsRect.top = mWindowHeightSpacing / 2 + mCellSize * 2;
		mLeftPointsRect.bottom = mLeftPointsRect.top + mCellSize * 6;
		mLeftPointsRect.left = horizontalCenter - mCellSize * 5;
		mLeftPointsRect.right = mLeftPointsRect.left + mCellSize;

		mRightPointsRect.top = mWindowHeightSpacing / 2 + mCellSize * 2;
		mRightPointsRect.bottom = mRightPointsRect.top + mCellSize * 6;
		mRightPointsRect.left = horizontalCenter + mCellSize * 4;
		mRightPointsRect.right = mRightPointsRect.left + mCellSize;

		mLeftPlayerNameRect.top = mWindowHeightSpacing / 2;
		mLeftPlayerNameRect.bottom = mLeftPlayerNameRect.top + mCellSize;
		mLeftPlayerNameRect.left = mWindowWidthSpacing / 2 + mCellSize * .5f;
		mLeftPlayerNameRect.right = horizontalCenter - mCellSize * 3;

		mRightPlayerNameRect.top = mWindowHeightSpacing / 2;
		mRightPlayerNameRect.bottom = mRightPlayerNameRect.top + mCellSize;
		mRightPlayerNameRect.left = horizontalCenter + mCellSize * 3;
		mRightPlayerNameRect.right = mWindowWidth - mWindowWidthSpacing / 2 - mCellSize * .5f;

		mCenterlineRects.resize(CENTERLINE_DOTS);
		for (auto i = 0; i < CENTERLINE_DOTS; i++) {
			mCenterlineRects[i].top = mWindowHeightSpacing / 2 + (i * 2 + 0.5f) * mCellSize;
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
			auto oldLeftRelMovement = abs(oldCellSize) <= EPSILON ? 0.f : (mLeftPaddle[i].top - (oldHalfHeight - (2.5f * oldCellSize))) / oldCellSize;
			auto oldRightRelMovement = abs(oldCellSize) <= EPSILON ? 0.f : (mRightPaddle[i].top - (oldHalfHeight - (2.5f * oldCellSize))) / oldCellSize;
			auto ballRelMovementY = abs(oldCellSize) <= EPSILON ? 0.f : (mBall[i].top - (oldHalfHeight - (.5f * oldCellSize))) / oldCellSize;
			auto ballRelMovementX = abs(oldCellSize) <= EPSILON ? 0.f : (mBall[i].left - (oldHalfWidth - (.5f * oldCellSize))) / oldCellSize;

			mLeftPaddle[i].top = verticalCenter - (2.5f * mCellSize) + mCellSize * oldLeftRelMovement;
			mLeftPaddle[i].bottom = mLeftPaddle[i].top + 5 * mCellSize;
			mLeftPaddle[i].left = mWindowWidthSpacing / 2 + mCellSize;
			mLeftPaddle[i].right = mLeftPaddle[i].left + mCellSize;

			mRightPaddle[i].top = verticalCenter - (2.5f * mCellSize) + mCellSize * oldRightRelMovement;
			mRightPaddle[i].bottom = mRightPaddle[i].top + 5 * mCellSize;
			mRightPaddle[i].left = mWindowWidth - (2 * mCellSize + mWindowWidthSpacing / 2);
			mRightPaddle[i].right = mRightPaddle[i].left + mCellSize;

			mBall[i].top = verticalCenter - (.5f * mCellSize) + mCellSize * ballRelMovementY;
			mBall[i].bottom = mBall[i].top + mCellSize;
			mBall[i].left = horizontalCenter - (.5f * mCellSize) + mCellSize * ballRelMovementX;
			mBall[i].right = mBall[i].left + mCellSize;
		}
		mGameOverRect = {
			horizontalCenter - 9 * mCellSize,
			verticalCenter - 3 * mCellSize,
			horizontalCenter + 9 * mCellSize,
			verticalCenter + 3 * mCellSize
		};
		mGameOverBigTextRect = {
			horizontalCenter - 9 * mCellSize,
			verticalCenter - 3 * mCellSize,
			horizontalCenter + 9 * mCellSize,
			verticalCenter + 3 * mCellSize
		};
		mGameOverSmallTextRect = {
			horizontalCenter - 10 * mCellSize,
			verticalCenter + mCellSize,
			horizontalCenter + 10 * mCellSize,
			verticalCenter + 2 * mCellSize
		};
	}

	void ResetMovingObjects() {
		// precalculate the half window dimensions.
		auto halfWidth = mWindowWidth / 2;
		auto halfHeight = mWindowHeight / 2;

		// reset both buffers for each moving object.
		for (auto i = 0; i < 2; i++) {
			mLeftPaddle[i].top = halfHeight - (2.5f * mCellSize);
			mLeftPaddle[i].bottom = mLeftPaddle[i].top + 5 * mCellSize;
			mLeftPaddle[i].left = mWindowWidthSpacing / 2 + mCellSize;
			mLeftPaddle[i].right = mLeftPaddle[i].left + mCellSize;

			mRightPaddle[i].top = halfHeight - (2.5f * mCellSize);
			mRightPaddle[i].bottom = mRightPaddle[i].top + 5 * mCellSize;
			mRightPaddle[i].left = mWindowWidth - (2 * mCellSize + mWindowWidthSpacing / 2);
			mRightPaddle[i].right = mRightPaddle[i].left + mCellSize;

			mBall[i].top = halfHeight - (.5f * mCellSize);
			mBall[i].bottom = mBall[i].top + mCellSize;
			mBall[i].left = halfWidth - (.5f * mCellSize);
			mBall[i].right = mBall[i].left + mCellSize;
		}

		// reset ball velocity.
		mBallVelocity = (mWindowHeight - mWindowHeightSpacing) / 30 / 4;
	}

	void RandomizeBallDirection() {
		auto x = -1.f + (2.f * util::GetRandomIntBetween(0, 1));
		auto y = -1.f + (2.f * util::GetRandomIntBetween(0, 1));
		mBallDirection = Vector2(x, y);
		mBallDirection.Normalize();
	}

	void GamepadAdded(Object^ o, Gamepad^ gamepad) {
		critical_section::scoped_lock lock{ mControllersLock };
		if (mLeftPlayerController == nullptr) {
			mLeftPlayerController = gamepad;
			auto user = gamepad->User;
			mLeftPlayer->SetName(((Platform::String^)create_task(user->GetPropertyAsync(KnownUserProperties::AccountName)).get())->Data());
		} else if (mRightPlayerController == nullptr) {
			mRightPlayerController = gamepad;
			auto user = gamepad->User;
			mRightPlayer->SetName(((Platform::String^)create_task(user->GetPropertyAsync(KnownUserProperties::AccountName)).get())->Data());
		}
	}

	void GamepadRemoved(Object^ o, Gamepad^ gamepad) {
		critical_section::scoped_lock lock{ mControllersLock };
		if (mLeftPlayerController == gamepad) {
			mLeftPlayerController = nullptr;
			mLeftPlayer->ResetName();
			mLeftPaddleVelocity.y = 0.f;
			Pause();
		} else if (mRightPlayerController == gamepad) {
			mRightPlayerController = nullptr;
			mRightPlayer->ResetName();
			mRightPaddleVelocity.y = 0.f;
			Pause();
		}
	}

	void Pause() {
		// TODO
	}

	void CheckKeyboard() {
		assert(mKeyboard != nullptr);
		if (!mKeyboard->IsConnected())
			return;

		mKeyboardTracker.Update(mKeyboard->GetState());

		if (IsGameOver()) {
			if (mKeyboardTracker.IsKeyReleased(Keyboard::Keys::Enter)) {
				ResetGame();
			}
		} else {
			if (mKeyboardTracker.IsKeyPressed(Keyboard::Keys::Up)) {
				mRightPaddleVelocity.y = -mPaddleVelocity;
			} else if (mKeyboardTracker.IsKeyReleased(Keyboard::Keys::Up)) {
				mRightPaddleVelocity.y = max(mRightPaddleVelocity.y, 0.f);
			}

			if (mKeyboardTracker.IsKeyPressed(Keyboard::Keys::Down)) {
				mRightPaddleVelocity.y = mPaddleVelocity;
			} else if (mKeyboardTracker.IsKeyReleased(Keyboard::Keys::Down)) {
				mRightPaddleVelocity.y = min(mRightPaddleVelocity.y, 0.f);
			}

			if (mKeyboardTracker.IsKeyPressed(Keyboard::Keys::W)) {
				mLeftPaddleVelocity.y = -mPaddleVelocity;
			} else if (mKeyboardTracker.IsKeyReleased(Keyboard::Keys::W)) {
				mLeftPaddleVelocity.y = max(mLeftPaddleVelocity.y, 0.f);
			}

			if (mKeyboardTracker.IsKeyPressed(Keyboard::Keys::S)) {
				mLeftPaddleVelocity.y = mPaddleVelocity;
			} else if (mKeyboardTracker.IsKeyReleased(Keyboard::Keys::S)) {
				mLeftPaddleVelocity.y = min(mLeftPaddleVelocity.y, 0.f);
			}
		}
	}

	void CheckGamepads() {
		critical_section::scoped_lock lock{ mControllersLock };
		if (mLeftPlayerController != nullptr) {
			auto reading = mLeftPlayerController->GetCurrentReading();
			mLeftPaddleVelocity.y = abs(reading.LeftThumbstickY) < GAMEPAD_DEADZONE ? 0.f : static_cast<float>(reading.LeftThumbstickY) * -mPaddleVelocity;
			if (IsGameOver()) {
				if (GamepadButtons::X == (reading.Buttons & GamepadButtons::X)) {
					ResetGame();
				}
			}
		}
		if (mRightPlayerController != nullptr) {
			auto reading = mRightPlayerController->GetCurrentReading();
			mRightPaddleVelocity.y = abs(reading.LeftThumbstickY) < GAMEPAD_DEADZONE ? 0.f : static_cast<float>(reading.LeftThumbstickY) * -mPaddleVelocity;
			if (IsGameOver()) {
				if (GamepadButtons::X == (reading.Buttons & GamepadButtons::X)) {
					ResetGame();
				}
			}
		}
	}

	void CheckInput() {
		CheckKeyboard();
		CheckGamepads();
	}

	void ClearPoints() {
		mLeftPlayer->SetScore(0);
		mRightPlayer->SetScore(0);
	}

	void StartCountdown() {
		mCountdown = COUNTDOWN_MS;
	}

	void ResetGame() {
		ClearPoints();
		StartCountdown();
		ResetMovingObjects();
		RandomizeBallDirection();
	}

	bool IsGameOver() {
		return mLeftPlayer->GetScore() >= POINT_TARGET || mRightPlayer->GetScore() >= POINT_TARGET;
	}

	void Update(int dt) {
		// don't update anything while the game is over.
		if (IsGameOver()) {
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
		auto leftPaddlePosition = mLeftPaddle[prevBufferIdx];
		auto rightPaddlePosition = mRightPaddle[prevBufferIdx];
		leftPaddlePosition.Move(mLeftPaddleVelocity.x, mLeftPaddleVelocity.y);
		rightPaddlePosition.Move(mRightPaddleVelocity.x, mRightPaddleVelocity.y);
		mLeftPaddle[mBufferIdx] = leftPaddlePosition;
		mRightPaddle[mBufferIdx] = rightPaddlePosition;

		// check that the left paddle stays between the top and bottom wall.
		if (mLeftPaddle[mBufferIdx].Collides(mBottomWall.GetRect())) {
			auto paddleHeight = mLeftPaddle[mBufferIdx].bottom - mLeftPaddle[mBufferIdx].top;
			mLeftPaddle[mBufferIdx].bottom = mBottomWall.GetY();
			mLeftPaddle[mBufferIdx].top = mBottomWall.GetY() - paddleHeight;
		} else if (mLeftPaddle[mBufferIdx].Collides(mTopWall.GetRect())) {
			auto paddleHeight = mLeftPaddle[mBufferIdx].bottom - mLeftPaddle[mBufferIdx].top;
			mLeftPaddle[mBufferIdx].bottom = mTopWall.GetY() + mTopWall.GetHeight() + paddleHeight;
			mLeftPaddle[mBufferIdx].top = mTopWall.GetY() + mTopWall.GetHeight();
		}

		// check that the right paddle stays between the top and bottom wall.
		if (mRightPaddle[mBufferIdx].Collides(mBottomWall.GetRect())) {
			auto paddleHeight = mRightPaddle[mBufferIdx].bottom - mRightPaddle[mBufferIdx].top;
			mRightPaddle[mBufferIdx].bottom = mBottomWall.GetY();
			mRightPaddle[mBufferIdx].top = mBottomWall.GetY() - paddleHeight;
		} else if (mRightPaddle[mBufferIdx].Collides(mTopWall.GetRect())) {
			auto paddleHeight = mRightPaddle[mBufferIdx].bottom - mRightPaddle[mBufferIdx].top;
			mRightPaddle[mBufferIdx].bottom = mTopWall.GetY() + mTopWall.GetHeight() + paddleHeight;
			mRightPaddle[mBufferIdx].top = mTopWall.GetY() + mTopWall.GetHeight();
		}

		/*
		// TODO temporary test solution
		mBallDirection = XMVector2Normalize(XMVectorSet(.5f, .5f, 0.f, 0.f));
		auto tBall = 1.f;
		auto ballPosition = mBall[prevBufferIdx];
		auto movement = XMVectorScale(mBallDirection, mBallVelocity);
		movement = XMVectorScale(movement, tBall);
		auto hitTime = 0.f;
		auto hitNormal = XMVectorSet(0.f, 0.f, 0.f, 0.f);
		if (mBallDirection.m128_f32[1] > 0.f && Intersect(ballPosition, mBottomWallRect, movement, XMVECTOR(), hitTime, hitNormal)) {
			mBallDirection = XMVectorSet(0.f, 0.f, 0.f, 0.f);
			return;
		}
		ballPosition = MoveAABB(ballPosition, movement);
		mBall[mBufferIdx] = ballPosition;
		*/
		auto tBall = static_cast<float>(dt);
		auto ballPosition = mBall[prevBufferIdx];
		while (tBall > 0.f) {
			// create a movement vector scaled with the time left for the ball.
			auto movement = XMVectorScale(mBallDirection, mBallVelocity * tBall);
			// movement = XMVectorScale(movement, tBall);

			auto hitTime = 0.f;
			auto hitNormal = XMVectorSet(0.f, 0.f, 0.f, 0.f);
			if (mBallDirection.y < 0.f && Intersect(ballPosition, mTopWall.GetRect(), movement, XMVECTOR(), hitTime, hitNormal)) {
				// move the ball straight to the hit point.
				// TODO old... movement = XMVectorScale(movement, hitTime);
				// movement = XMVectorScale(mBallDirection, mBallVelocity * tBall * hitTime);
				movement = XMVectorScale(movement, hitTime);
				ballPosition.Move(movement.m128_f32[0], movement.m128_f32[1]);

				// inverse balls vertical movement direction.
				mBallDirection.y = -mBallDirection.y;

				mBallDirection = XMVECTOR();
				tBall = 0.f;
				// ballPosition = mBall[prevBufferIdx];

				/*
				// apply a small nudge to make the ball to leave collision area.
				movement = XMVectorScale(mBallDirection, mBallVelocity);
				movement = XMVectorScale(movement, NUDGE);
				ballPosition = MoveAABB(ballPosition, movement);

				// decrease the amount of usable time for ball movement.
				tBall -= hitTime;
				mBeepSound.Play();
				*/
			} else if (mBallDirection.y > 0.f && Intersect(ballPosition, mBottomWall.GetRect(), movement, XMVECTOR(), hitTime, hitNormal)) {
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
			} else if (mBallDirection.y < 0.f && Intersect(ballPosition, mLeftPaddle[prevBufferIdx], movement, mLeftPaddleVelocity, hitTime, hitNormal)) {
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
			} else if (mBallDirection.y > 0.f && Intersect(ballPosition, mRightPaddle[prevBufferIdx], movement, mLeftPaddleVelocity, hitTime, hitNormal)) {
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
		mBall[mBufferIdx] = ballPosition;

		// check whether the ball has reached a goal.
		if (mLeftGoalRect.Contains(mBall[mBufferIdx])) {
			handleGoal(PLAYER_RIGHT);
		} else if (mRightGoalRect.Contains(mBall[mBufferIdx])) {
			handleGoal(PLAYER_LEFT);
		}
	}

	void handleGoal(uint8_t player) {
		if (player == PLAYER_LEFT) {
			mLeftPlayer->IncrementScore();
		} else {
			mRightPlayer->IncrementScore();
		}
		mCountdown = COUNTDOWN_MS;
		ResetMovingObjects();
		RandomizeBallDirection();
	}

	void Render(float alpha) {
		mGraphics->BeginDrawAndClear();

		mGraphics->DrawWhiteRects(mCenterlineRects);
		mGraphics->DrawWhiteRect(mTopWall.GetRect());
		mGraphics->DrawWhiteRect(mBottomWall.GetRect());
		mGraphics->DrawWhiteBigText(std::to_wstring(mLeftPlayer->GetScore()), mLeftPointsRect);
		mGraphics->DrawWhiteBigText(std::to_wstring(mRightPlayer->GetScore()), mRightPointsRect);
		mGraphics->DrawBlackSmallText(mLeftPlayer->GetName(), mLeftPlayerNameRect);
		mGraphics->DrawBlackSmallText(mRightPlayer->GetName(), mRightPlayerNameRect);
		mGraphics->DrawWhiteRect(mRightGoalRect);

		// dynamic objects

		auto prevBufferIdx = mBufferIdx == 0 ? 1 : 0;

		mGraphics->DrawWhiteRect(geometry::Rectangle::Lerp(
			mLeftPaddle[prevBufferIdx],
			mLeftPaddle[mBufferIdx],
			alpha));
		mGraphics->DrawWhiteRect(geometry::Rectangle::Lerp(
			mRightPaddle[prevBufferIdx],
			mRightPaddle[mBufferIdx],
			alpha));
		mGraphics->DrawWhiteRect(geometry::Rectangle::Lerp(
			mBall[prevBufferIdx],
			mBall[mBufferIdx],
			alpha));

		// draw the game over box if the game is over
		if (IsGameOver()) {
			mGraphics->DrawWhiteRect(mGameOverRect);
			mGraphics->DrawBlackMediumText(GAME_OVER_TITLE, mGameOverBigTextRect);
			mGraphics->DrawBlackSmallText(GAME_OVER_DESCRIPTON, mGameOverSmallTextRect);
		}
		mGraphics->EndDrawAndPresent();
	}
private:
	std::unique_ptr<Player> mLeftPlayer;
	std::unique_ptr<Player> mRightPlayer;

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

	Gamepad^ mLeftPlayerController;
	Gamepad^ mRightPlayerController;
	critical_section	mControllersLock;

	Vector2 mLeftPaddleVelocity;
	Vector2 mRightPaddleVelocity;

	Vector2 mBallDirection;

	std::unique_ptr<graphics::Graphics> mGraphics;

	std::vector<geometry::Rectangle> mCenterlineRects;

	Entity mTopWall;
	Entity mBottomWall;

	geometry::Rectangle mLeftPointsRect;
	geometry::Rectangle mRightPointsRect;
	geometry::Rectangle mLeftPlayerNameRect;
	geometry::Rectangle mRightPlayerNameRect;
	geometry::Rectangle mLeftPaddle[2];
	geometry::Rectangle mRightPaddle[2];
	geometry::Rectangle mBall[2];
	geometry::Rectangle mLeftGoalRect;
	geometry::Rectangle mRightGoalRect;

	geometry::Rectangle mGameOverRect;
	geometry::Rectangle mGameOverBigTextRect;
	geometry::Rectangle mGameOverSmallTextRect;

	int mBufferIdx = 0;

	std::unique_ptr<Keyboard> mKeyboard;
	Keyboard::KeyboardStateTracker mKeyboardTracker;

	std::unique_ptr<AudioEngine> mAudioEngine;
	std::unique_ptr<SoundEffect> mBeepSound;
};

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	CoreApplication::Run(ref new Pong());
	return 0;
}