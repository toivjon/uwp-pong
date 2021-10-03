#include "pch.hpp"
#include "game.hpp"

#include <array>
#include <random>

using namespace winrt::Windows::Gaming::Input;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::System;

// The center of the courtyard in y-axis.
static const auto CenterY = .5f;

// The center of the courtyard in x-axis.
static const auto CenterX = .5f;

// A constant presenting the center of the courtyard.
static const auto Center = Vec2f{ CenterX, CenterY };

// The amount of update ticks to wait before ball is launched after a game reset.
static const auto CountdownTicks = 50;

// The initial velocity of the ball.
static const auto BallInitialVelocity = .0004f;

// The amount of velocity to multiply during each ball-paddle collision.
static const auto BallVelocityMultiplier = 1.1f;

// The amount of additional nudge to add to collision response to separate collded objects.
static const auto Nudge = .001f;

// The velocity of player controlled paddles.
static const auto PaddleVelocity = .001f;

// Build a randomly selected direction vector from 45, 135, 225 and 315 degrees.
inline auto NewRandomDirection() -> Vec2f {
	static std::default_random_engine rng;
	static std::uniform_int_distribution<std::mt19937::result_type> dist(0, 3);
	static const std::array<Vec2f, 4> dirs = {
		Vec2f{BallInitialVelocity, BallInitialVelocity},
		Vec2f{BallInitialVelocity, -BallInitialVelocity},
		Vec2f{-BallInitialVelocity, BallInitialVelocity},
		Vec2f{-BallInitialVelocity, -BallInitialVelocity}
	};
	return dirs[dist(rng)];
}

auto Intersects(const Rectangle& a, const Rectangle& b) -> bool {
	const auto amin = a.position - a.extent;
	const auto amax = a.position + a.extent;
	const auto bmin = b.position - b.extent;
	const auto bmax = b.position + b.extent;
	return amin.x <= bmax.x && amax.x >= bmin.x && amin.y <= bmax.y && amax.y >= bmin.y;
}

struct Intersection {
	bool  collides;
	float time;
};

auto Intersects(const Rectangle& a, const Rectangle& b, const Vec2f& va, const Vec2f& vb) -> Intersection {
	Intersection intersection = {};
	intersection.collides = false;
	intersection.time = 0.f;

	// Exit early whether boxes initially collide.
	if (Intersects(a, b)) {
		intersection.collides = true;
		intersection.time = 0.f;
		return intersection;
	}

	const auto amin = a.position - a.extent;
	const auto amax = a.position + a.extent;
	const auto bmin = b.position - b.extent;
	const auto bmax = b.position + b.extent;

	// We will use relative velocity where 'a' is treated as stationary.
	const auto v = vb - va;

	// Initialize times for the first and last contact.
	auto tmin = -FLT_MAX;
	auto tmax = FLT_MAX;

	// Find the first and last contact from x-axis.
	if (v.x < .0f) {
		if (bmax.x < amin.x) return intersection;
		if (amax.x < bmin.x) tmin = std::max((amax.x - bmin.x) / v.x, tmin);
		if (bmax.x > amin.x) tmax = std::min((amin.x - bmax.x) / v.x, tmax);
	}
	if (v.x > .0f) {
		if (bmin.x > amax.x) return intersection;
		if (bmax.x < amin.x) tmin = std::max((amin.x - bmax.x) / v.x, tmin);
		if (amax.x > bmin.x) tmax = std::min((amax.x - bmin.x) / v.x, tmax);
	}
	if (tmin > tmax) return intersection;

	// Find the first and last contact from y-axis.
	if (v.y < .0f) {
		if (bmax.y < amin.y) return intersection;
		if (amax.y < bmin.y) tmin = std::max((amax.y - bmin.y) / v.y, tmin);
		if (bmax.y > amin.y) tmax = std::min((amin.y - bmax.y) / v.y, tmax);
	}
	if (v.y > .0f) {
		if (bmin.y > amax.y) return intersection;
		if (bmax.y < amin.y) tmin = std::max((amin.y - bmax.y) / v.y, tmin);
		if (amax.y > bmin.y) tmax = std::min((amax.y - bmin.y) / v.y, tmax);
	}
	if (tmin > tmax) return intersection;

	intersection.collides = (tmin >= 0.f && tmin <= 1.f);
	intersection.time = tmin;
	return intersection;
}

Game::Game(const Renderer::Ptr& renderer, Audio::Ptr& audio) : mDialogVisible(true), mAudio(audio) {
	mDialogBackground.extent = { 0.375f, 0.40f };
	mDialogBackground.position = { Center };
	mDialogBackground.brush = renderer->getWhiteBrush();

	mDialogForeground.extent = { 0.35f, 0.375f };
	mDialogForeground.position = { Center };
	mDialogForeground.brush = renderer->getBlackBrush();

	mDialogTopic.brush = renderer->getWhiteBrush();
	mDialogTopic.text = L"UWP Pong";
	mDialogTopic.position = { CenterX, .3f };
	mDialogTopic.fontSize = .1f;

	mDialogDescription.brush = renderer->getWhiteBrush();
	mDialogDescription.text = L"Press X key or button to start a game";
	mDialogDescription.position = { CenterX, .6f };
	mDialogDescription.fontSize = .05f;

	mBall.extent = { .0115f, .015f };
	mBall.position = Center;
	mBall.brush = renderer->getWhiteBrush();

	mUpperWall.extent = { .5f, .015f };
	mUpperWall.position = { CenterX, .015f };
	mUpperWall.brush = renderer->getWhiteBrush();

	mLowerWall.extent = mUpperWall.extent;
	mLowerWall.position = { CenterX, .985f };
	mLowerWall.brush = renderer->getWhiteBrush();

	mLeftPaddle.extent = { .0125f, .075f };
	mLeftPaddle.position = { .05f, CenterY };
	mLeftPaddle.brush = renderer->getWhiteBrush();
	mLeftPaddle.velocity = { 0.f, 0.f };

	mRightPaddle.extent = mLeftPaddle.extent;
	mRightPaddle.position = { .95f, CenterY };
	mRightPaddle.brush = renderer->getWhiteBrush();
	mRightPaddle.velocity = { 0.f, 0.f };

	mLeftScore.text = std::to_wstring(mP1Score);
	mLeftScore.position = { .35f, .025f };
	mLeftScore.brush = renderer->getWhiteBrush();
	mLeftScore.fontSize = .27f;

	mRightScore.text = std::to_wstring(mP2Score);
	mRightScore.position = { .65f, .025f };
	mRightScore.brush = renderer->getWhiteBrush();
	mRightScore.fontSize = .27f;

	mLeftGoal.extent = { .5f, .5f };
	mLeftGoal.position = { -.5f - mBall.extent.x * 4.f, CenterY };
	mLeftGoal.brush = renderer->getWhiteBrush();

	mRightGoal.extent = mLeftGoal.extent;
	mRightGoal.position = { 1.5f + mBall.extent.x * 4.f, CenterY };
	mRightGoal.brush = renderer->getWhiteBrush();

	mBeepSound = mAudio->createSound(L"Assets/beep.wav");

	newGame();
}

auto Game::detectCollision(float deltaMS) const -> CollisionResult {
	auto result = CollisionResult{};
	result.hasHit = false;
	result.hitTime = FLT_MAX;
	auto hit = Intersects(mBall, mLeftPaddle, mBall.velocity * deltaMS, mLeftPaddle.velocity * deltaMS);
	if (hit.collides && hit.time < result.hitTime) {
		result.hitTime = hit.time;
		result.hasHit = true;
		result.candidate.lhs = CandidateType::BALL;
		result.candidate.rhs = CandidateType::LPADDLE;
	}
	hit = Intersects(mBall, mRightPaddle, mBall.velocity * deltaMS, mRightPaddle.velocity * deltaMS);
	if (hit.collides && hit.time < result.hitTime) {
		result.hitTime = hit.time;
		result.hasHit = true;
		result.candidate.lhs = CandidateType::BALL;
		result.candidate.rhs = CandidateType::RPADDLE;
	}
	hit = Intersects(mBall, mUpperWall, mBall.velocity * deltaMS, { 0.f, 0.f });
	if (hit.collides && hit.time < result.hitTime) {
		result.hitTime = hit.time;
		result.hasHit = true;
		result.candidate.lhs = CandidateType::BALL;
		result.candidate.rhs = CandidateType::TWALL;
	}
	hit = Intersects(mBall, mLowerWall, mBall.velocity * deltaMS, { 0.f, 0.f });
	if (hit.collides && hit.time < result.hitTime) {
		result.hitTime = hit.time;
		result.hasHit = true;
		result.candidate.lhs = CandidateType::BALL;
		result.candidate.rhs = CandidateType::BWALL;
	}
	hit = Intersects(mBall, mLeftGoal, mBall.velocity * deltaMS, { 0.f, 0.f });
	if (hit.collides && hit.time < result.hitTime) {
		result.hitTime = hit.time;
		result.hasHit = true;
		result.candidate.lhs = CandidateType::BALL;
		result.candidate.rhs = CandidateType::LGOAL;
	}
	hit = Intersects(mBall, mRightGoal, mBall.velocity * deltaMS, { 0.f, 0.f });
	if (hit.collides && hit.time < result.hitTime) {
		result.hitTime = hit.time;
		result.hasHit = true;
		result.candidate.lhs = CandidateType::BALL;
		result.candidate.rhs = CandidateType::RGOAL;
	}
	hit = Intersects(mLeftPaddle, mUpperWall, mLeftPaddle.velocity * deltaMS, { 0.f, 0.f });
	if (hit.collides && hit.time < result.hitTime) {
		result.hitTime = hit.time;
		result.hasHit = true;
		result.candidate.lhs = CandidateType::LPADDLE;
		result.candidate.rhs = CandidateType::TWALL;
	}
	hit = Intersects(mLeftPaddle, mLowerWall, mLeftPaddle.velocity * deltaMS, { 0.f, 0.f });
	if (hit.collides && hit.time < result.hitTime) {
		result.hitTime = hit.time;
		result.hasHit = true;
		result.candidate.lhs = CandidateType::LPADDLE;
		result.candidate.rhs = CandidateType::BWALL;
	}
	hit = Intersects(mRightPaddle, mUpperWall, mRightPaddle.velocity * deltaMS, { 0.f,0.f });
	if (hit.collides && hit.time < result.hitTime) {
		result.hitTime = hit.time;
		result.hasHit = true;
		result.candidate.lhs = CandidateType::RPADDLE;
		result.candidate.rhs = CandidateType::TWALL;
	}
	hit = Intersects(mRightPaddle, mLowerWall, mRightPaddle.velocity * deltaMS, { 0.f,0.f });
	if (hit.collides && hit.time < result.hitTime) {
		result.hitTime = hit.time;
		result.hasHit = true;
		result.candidate.lhs = CandidateType::RPADDLE;
		result.candidate.rhs = CandidateType::BWALL;
	}
	return result;
}

void Game::update(std::chrono::milliseconds delta) {
	// Skip the update whether the game has been just launched or the game has ended.
	if (mDialogVisible) {
		return;
	}

	// Skip the update whether the countdown is still in progress.
	if (mCountdown > 0) {
		mCountdown--;
		return;
	}

	// Get the time (in milliseconds) we must consume during this simulation step.
	auto deltaMS = static_cast<float>(delta.count());

	// Apply the keyboard and gamepad input to paddle velocities.
	switch (mP1MoveDirection) {
	case MoveDirection::NONE:
		mLeftPaddle.velocity.y = 0.f;
		break;
	case MoveDirection::UP:
		mLeftPaddle.velocity.y = -PaddleVelocity;
		break;
	case MoveDirection::DOWN:
		mLeftPaddle.velocity.y = PaddleVelocity;
		break;
	}
	switch (mP2MoveDirection) {
	case MoveDirection::NONE:
		mRightPaddle.velocity.y = 0.f;
		break;
	case MoveDirection::UP:
		mRightPaddle.velocity.y = -PaddleVelocity;
		break;
	case MoveDirection::DOWN:
		mRightPaddle.velocity.y = PaddleVelocity;
		break;
	}

	// TODO A temporary solution which should be handled in a more elegant way.
	auto mustStartGame = false;
	do {
		// Perform collision detection to find out the first collision.
		const auto collision = detectCollision(deltaMS);
		if (!collision.hasHit) {
			mLeftPaddle.position = mLeftPaddle.position + mLeftPaddle.velocity * deltaMS;
			mRightPaddle.position = mRightPaddle.position + mRightPaddle.velocity * deltaMS;
			mBall.position = mBall.position + mBall.velocity * deltaMS;
			break;
		}

		// Consume simulation time and resolve collisions based on the first collision.
		const auto collisionMS = collision.hitTime * deltaMS;
		deltaMS -= collisionMS;

		// Apply movement to dynamic entities.
		mBall.position += mBall.velocity * collisionMS;
		mLeftPaddle.position += mLeftPaddle.velocity * collisionMS;
		mRightPaddle.position += mRightPaddle.velocity * collisionMS;

		switch (collision.candidate.lhs) {
		case CandidateType::BALL:
			switch (collision.candidate.rhs) {
			case CandidateType::BWALL:
				mBall.position.y = mLowerWall.position.y - mLowerWall.extent.y - mBall.extent.y - Nudge;
				mBall.velocity.y = -mBall.velocity.y;
				mAudio->playSound(mBeepSound);
				break;
			case CandidateType::TWALL:
				mBall.position.y = mUpperWall.position.y + mUpperWall.extent.y + mBall.extent.y + Nudge;
				mBall.velocity.y = -mBall.velocity.y;
				mAudio->playSound(mBeepSound);
				break;
			case CandidateType::LGOAL:
				mP2Score++;
				if (mP2Score >= 10) {
					mDialogVisible = true;
					mDialogTopic.text = L"Right player wins!";
				} else {
					mRightScore.text = std::to_wstring(mP2Score);
				}
				mustStartGame = true;
				break;
			case CandidateType::RGOAL:
				mP1Score++;
				if (mP1Score >= 10) {
					mDialogVisible = true;
					mDialogTopic.text = L"Left player wins!";
				} else {
					mLeftScore.text = std::to_wstring(mP1Score);
				}
				mustStartGame = true;
				break;
			case CandidateType::LPADDLE: {
				mBall.position.x = mLeftPaddle.position.x + mLeftPaddle.extent.x + mBall.extent.x + Nudge;
				mBall.velocity.x = -mBall.velocity.x;
				mBall.velocity = mBall.velocity * BallVelocityMultiplier;
				mAudio->playSound(mBeepSound);
				break;
			}
			case CandidateType::RPADDLE:
				mBall.position.x = mRightPaddle.position.x - mRightPaddle.extent.x - mBall.extent.x - Nudge;
				mBall.velocity.x = -mBall.velocity.x;
				mBall.velocity = mBall.velocity * BallVelocityMultiplier;
				mAudio->playSound(mBeepSound);
				break;
			}
			break;
		case CandidateType::LPADDLE:
			switch (collision.candidate.rhs) {
			case CandidateType::BWALL:
				mLeftPaddle.position.y = mLowerWall.position.y - mLowerWall.extent.y - mLeftPaddle.extent.y - Nudge;
				break;
			case CandidateType::TWALL:
				mLeftPaddle.position.y = mUpperWall.position.y + mUpperWall.extent.y + mLeftPaddle.extent.y + Nudge;
				break;
			}
			mLeftPaddle.velocity.y = 0.f;
			break;
		case CandidateType::RPADDLE:
			switch (collision.candidate.rhs) {
			case CandidateType::BWALL:
				mRightPaddle.position.y = mLowerWall.position.y - mLowerWall.extent.y - mRightPaddle.extent.y - Nudge;
				break;
			case CandidateType::TWALL:
				mRightPaddle.position.y = mUpperWall.position.y + mUpperWall.extent.y + mRightPaddle.extent.y + Nudge;
				break;
			}
			mRightPaddle.velocity.y = 0.f;
			break;
		}
		if (mustStartGame) {
			break;
		}
	} while (true);

	if (mustStartGame) {
		newRound();
	}
}

void Game::render(const Renderer::Ptr& renderer) const {
	// Base game entities are always shown.
	renderer->draw(mLeftScore);
	renderer->draw(mRightScore);
	renderer->draw(mBall);
	renderer->draw(mUpperWall);
	renderer->draw(mLowerWall);
	renderer->draw(mLeftPaddle);
	renderer->draw(mRightPaddle);

	// Dialog stuff is shown only on game enter or end game.
	if (mDialogVisible) {
		renderer->draw(mDialogBackground);
		renderer->draw(mDialogForeground);
		renderer->draw(mDialogTopic);
		renderer->draw(mDialogDescription);
	}
}

void Game::onKeyDown(const KeyEventArgs& args) {
	switch (args.VirtualKey()) {
	case VirtualKey::Up:
		mP2MoveDirection = MoveDirection::UP;
		// TODO mRightPaddle.velocity.y = -PaddleVelocity;
		break;
	case VirtualKey::Down:
		mP2MoveDirection = MoveDirection::DOWN;
		// TODO mRightPaddle.velocity.y = PaddleVelocity;
		break;
	case VirtualKey::W:
		mP1MoveDirection = MoveDirection::UP;
		// TODO mLeftPaddle.velocity.y = -PaddleVelocity;
		break;
	case VirtualKey::S:
		mP1MoveDirection = MoveDirection::DOWN;
		// TODO mLeftPaddle.velocity.y = PaddleVelocity;
		break;
	case VirtualKey::X:
		if (mDialogVisible) {
			newGame();
			mDialogVisible = false;
		}
		break;
	}
}

void Game::onKeyUp(const KeyEventArgs& args) {
	switch (args.VirtualKey()) {
	case VirtualKey::Up:
		mP2MoveDirection = (mP2MoveDirection == MoveDirection::UP ? MoveDirection::NONE : mP2MoveDirection);
		// TODO mRightPaddle.velocity.y = std::max(0.f, mRightPaddle.velocity.y);
		break;
	case VirtualKey::Down:
		mP2MoveDirection = (mP2MoveDirection == MoveDirection::DOWN ? MoveDirection::NONE : mP2MoveDirection);
		// TODO mRightPaddle.velocity.y = std::min(0.f, mRightPaddle.velocity.y);
		break;
	case VirtualKey::W:
		mP1MoveDirection = (mP1MoveDirection == MoveDirection::UP ? MoveDirection::NONE : mP1MoveDirection);
		// TODO mLeftPaddle.velocity.y = std::max(0.f, mLeftPaddle.velocity.y);
		break;
	case VirtualKey::S:
		mP1MoveDirection = (mP1MoveDirection == MoveDirection::DOWN ? MoveDirection::NONE : mP1MoveDirection);
		// mLeftPaddle.velocity.y = std::min(0.f, mLeftPaddle.velocity.y);
		break;
	}
}

void Game::newRound() {
	mBall.position = Center;
	mBall.velocity = NewRandomDirection();
	mLeftPaddle.position.y = CenterY;
	mRightPaddle.position.y = CenterY;
	mCountdown = CountdownTicks;
}

void Game::newGame() {
	mP1Score = 0;
	mP2Score = 0;
	mRightScore.text = std::to_wstring(mP2Score);
	mLeftScore.text = std::to_wstring(mP1Score);
	newRound();
}

void Game::onReadGamepad(int player, const GamepadReading& reading) {
	if (mDialogVisible) {
		if (GamepadButtons::X == (reading.Buttons & GamepadButtons::X)) {
			newGame();
			mDialogVisible = false;
		}
	} else {
		static const auto DeadZone = .25f;
		auto velocity = 0.f;
		auto moveDirection = MoveDirection::NONE;
		if (reading.LeftThumbstickY > DeadZone) {
			moveDirection = MoveDirection::UP;
			// TODO velocity = -PaddleVelocity;
		} else if (reading.LeftThumbstickY < -DeadZone) {
			moveDirection = MoveDirection::DOWN;
			// velocity = PaddleVelocity;
		}
		if (player == 0) {
			mP1MoveDirection = moveDirection;
			// TODO mLeftPaddle.velocity.y = velocity;
		} else {
			mP2MoveDirection = moveDirection;
			// TODO mRightPaddle.velocity.y = velocity;
		}
	}
}