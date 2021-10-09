#include "pch.hpp"
#include "game.hpp"

#include <array>
#include <random>

using namespace winrt::Windows::Gaming::Input;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::System;

// Build a randomly selected direction vector from 45, 135, 225 and 315 degrees.
inline auto NewRandomDirection() -> Vec2f {
	constexpr auto BallInitialVelocity = .0004f;
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

Game::Game(Audio::Ptr& audio) : audio(audio) {
	state = std::make_shared<Game::DialogState>(L"Press X key or button to start a game");

	mBall.extent = { .0115f, .015f };
	mBall.position = {.5f, .5f};
	mBall.id = ObjectID::BALL;

	mTopWall.extent = { .5f, .015f };
	mTopWall.position = { .5f, .015f };
	mTopWall.id = ObjectID::TOP_WALL;

	mBottomWall.extent = mTopWall.extent;
	mBottomWall.position = { .5f, .985f };
	mBottomWall.id = ObjectID::BOTTOM_WALL;

	mLeftPaddle.extent = { .0125f, .075f };
	mLeftPaddle.position = { .05f, .5f};
	mLeftPaddle.velocity = { 0.f, 0.f };
	mLeftPaddle.id = ObjectID::LEFT_PADDLE;

	mRightPaddle.extent = mLeftPaddle.extent;
	mRightPaddle.position = { .95f, .5f};
	mRightPaddle.velocity = { 0.f, 0.f };
	mRightPaddle.id = ObjectID::RIGHT_PADDLE;

	mLeftScore.text = std::to_wstring(player1Score);
	mLeftScore.position = { .35f, .025f };
	mLeftScore.fontSize = .27f;

	mRightScore.text = std::to_wstring(player2Score);
	mRightScore.position = { .65f, .025f };
	mRightScore.fontSize = .27f;

	mLeftGoal.extent = { .5f, .5f };
	mLeftGoal.position = { -.5f - mBall.extent.x * 4.f, .5f };
	mLeftGoal.id = ObjectID::LEFT_GOAL;

	mRightGoal.extent = mLeftGoal.extent;
	mRightGoal.position = { 1.5f + mBall.extent.x * 4.f, .5f };
	mRightGoal.id = ObjectID::RIGHT_GOAL;

	mBeepSound = audio->createSound(L"Assets/beep.wav");
}

auto Game::detectCollision(float deltaMS) const -> Collision {
	auto result = Collision{};
	detectCollision(deltaMS, mBall, mLeftPaddle, result);
	detectCollision(deltaMS, mBall, mRightPaddle, result);
	detectCollision(deltaMS, mBall, mTopWall, result);
	detectCollision(deltaMS, mBall, mBottomWall, result);
	detectCollision(deltaMS, mBall, mLeftGoal, result);
	detectCollision(deltaMS, mBall, mRightGoal, result);
	detectCollision(deltaMS, mLeftPaddle, mTopWall, result);
	detectCollision(deltaMS, mLeftPaddle, mBottomWall, result);
	detectCollision(deltaMS, mRightPaddle, mTopWall, result);
	detectCollision(deltaMS, mRightPaddle, mBottomWall, result);
	return result;
}

void Game::detectCollision(float deltaMS, const Rectangle& r1, const Rectangle& r2, Collision& result) const {
	auto hit = detectCollision(deltaMS, r1, r2);
	if (hit.lhs != ObjectID::NONE && hit.time < result.time) {
		result.time = hit.time;
		result.lhs = r1.id;
		result.rhs = r2.id;
	}
}

auto Game::detectCollision(float deltaMS, const Rectangle& a, const Rectangle& b) const->Collision {
	auto collision = Collision{};

	const auto amin = a.position - a.extent;
	const auto amax = a.position + a.extent;
	const auto bmin = b.position - b.extent;
	const auto bmax = b.position + b.extent;

	// Exit early whether boxes initially collide.
	if (amin.x <= bmax.x && amax.x >= bmin.x && amin.y <= bmax.y && amax.y >= bmin.y) {
		collision.lhs = a.id;
		collision.rhs = b.id;
		collision.time = 0.f;
		return collision;
	}

	// We will use relative velocity where 'a' is treated as stationary.
	const auto v = (b.velocity - a.velocity) * deltaMS;

	// Initialize times for the first and last contact.
	auto tmin = -FLT_MAX;
	auto tmax = FLT_MAX;

	// Find the first and last contact from x-axis.
	if (v.x < .0f) {
		if (bmax.x < amin.x) return collision;
		if (amax.x < bmin.x) tmin = std::max((amax.x - bmin.x) / v.x, tmin);
		if (bmax.x > amin.x) tmax = std::min((amin.x - bmax.x) / v.x, tmax);
	}
	if (v.x > .0f) {
		if (bmin.x > amax.x) return collision;
		if (bmax.x < amin.x) tmin = std::max((amin.x - bmax.x) / v.x, tmin);
		if (amax.x > bmin.x) tmax = std::min((amax.x - bmin.x) / v.x, tmax);
	}
	if (tmin > tmax) return collision;

	// Find the first and last contact from y-axis.
	if (v.y < .0f) {
		if (bmax.y < amin.y) return collision;
		if (amax.y < bmin.y) tmin = std::max((amax.y - bmin.y) / v.y, tmin);
		if (bmax.y > amin.y) tmax = std::min((amin.y - bmax.y) / v.y, tmax);
	}
	if (v.y > .0f) {
		if (bmin.y > amax.y) return collision;
		if (bmax.y < amin.y) tmin = std::max((amin.y - bmax.y) / v.y, tmin);
		if (amax.y > bmin.y) tmax = std::min((amax.y - bmin.y) / v.y, tmax);
	}
	if (tmin > tmax) return collision;

	if (tmin >= 0.f && tmin <= 1.f) {
		collision.lhs = a.id;
		collision.rhs = b.id;
		collision.time = tmin;
	}
	return collision;
}

void Game::resolveCollision(const Collision& collision) {
	constexpr auto BallVelocityMultiplier = 1.1f;
	constexpr auto Nudge = .001f;
	switch (collision.lhs) {
	case ObjectID::BALL:
		switch (collision.rhs) {
		case ObjectID::BOTTOM_WALL:
			mBall.position.y = mBottomWall.position.y - mBottomWall.extent.y - mBall.extent.y - Nudge;
			mBall.velocity.y = -mBall.velocity.y;
			audio->playSound(mBeepSound);
			break;
		case ObjectID::TOP_WALL:
			mBall.position.y = mTopWall.position.y + mTopWall.extent.y + mBall.extent.y + Nudge;
			mBall.velocity.y = -mBall.velocity.y;
			audio->playSound(mBeepSound);
			break;
		case ObjectID::LEFT_GOAL:
			player2Score++;
			if (player2Score >= 10) {
				state = std::make_shared<DialogState>(L"Right player wins! Press X for rematch.");
			} else {
				mRightScore.text = std::to_wstring(player2Score);
				mNewRound = true;
			}
			break;
		case ObjectID::RIGHT_GOAL:
			player1Score++;
			if (player1Score >= 10) {
				state = std::make_shared<DialogState>(L"Left player wins! Press X for rematch.");
			} else {
				mLeftScore.text = std::to_wstring(player1Score);
				mNewRound = true;
			}
			break;
		case ObjectID::LEFT_PADDLE: {
			mBall.position.x = mLeftPaddle.position.x + mLeftPaddle.extent.x + mBall.extent.x + Nudge;
			mBall.velocity.x = -mBall.velocity.x;
			mBall.velocity = mBall.velocity * BallVelocityMultiplier;
			audio->playSound(mBeepSound);
			break;
		}
		case ObjectID::RIGHT_PADDLE:
			mBall.position.x = mRightPaddle.position.x - mRightPaddle.extent.x - mBall.extent.x - Nudge;
			mBall.velocity.x = -mBall.velocity.x;
			mBall.velocity = mBall.velocity * BallVelocityMultiplier;
			audio->playSound(mBeepSound);
			break;
		}
		break;
	case ObjectID::LEFT_PADDLE:
		switch (collision.rhs) {
		case ObjectID::BOTTOM_WALL:
			mLeftPaddle.position.y = mBottomWall.position.y - mBottomWall.extent.y - mLeftPaddle.extent.y - Nudge;
			break;
		case ObjectID::TOP_WALL:
			mLeftPaddle.position.y = mTopWall.position.y + mTopWall.extent.y + mLeftPaddle.extent.y + Nudge;
			break;
		}
		mLeftPaddle.velocity.y = 0.f;
		break;
	case ObjectID::RIGHT_PADDLE:
		switch (collision.rhs) {
		case ObjectID::BOTTOM_WALL:
			mRightPaddle.position.y = mBottomWall.position.y - mBottomWall.extent.y - mRightPaddle.extent.y - Nudge;
			break;
		case ObjectID::TOP_WALL:
			mRightPaddle.position.y = mTopWall.position.y + mTopWall.extent.y + mRightPaddle.extent.y + Nudge;
			break;
		}
		mRightPaddle.velocity.y = 0.f;
		break;
	}
}

Game::DialogState::DialogState(const std::wstring& descriptionText) {
	background.extent = { 0.375f, 0.40f };
	background.position = { .5f, .5f };

	foreground.extent = { 0.35f, 0.375f };
	foreground.position = { .5f, .5f };

	topic.text = L"UWP Pong";
	topic.position = { .5f, .3f };
	topic.fontSize = .1f;

	description.text = descriptionText;
	description.position = { .5f, .6f };
	description.fontSize = .05f;
}

void Game::DialogState::render(Game&, const Renderer::Ptr& renderer) {
	renderer->draw(renderer->getWhiteBrush(), background);
	renderer->draw(renderer->getBlackBrush(), foreground);
	renderer->draw(renderer->getWhiteBrush(), topic);
	renderer->draw(renderer->getWhiteBrush(), description);
}

void Game::DialogState::onKeyDown(Game& game, const winrt::Windows::UI::Core::KeyEventArgs& args) {
	if (args.VirtualKey() == VirtualKey::X) {
		startGame(game);
	}
}

void Game::DialogState::onReadGamepad(Game& game, int, const winrt::Windows::Gaming::Input::GamepadReading& reading) {
	if (GamepadButtons::X == (reading.Buttons & GamepadButtons::X)) {
		startGame(game);
	}
}

void Game::DialogState::startGame(Game& game) {
	game.player1Score = 0;
	game.player2Score = 0;
	game.mRightScore.text = std::to_wstring(game.player2Score);
	game.mLeftScore.text = std::to_wstring(game.player1Score);
	game.state = std::make_shared<CountdownState>(game);
}

Game::CountdownState::CountdownState(Game& game) {
	game.mBall.position = { .5f, .5f };
	game.mBall.velocity = NewRandomDirection();
	game.mLeftPaddle.position.y = .5f;
	game.mRightPaddle.position.y = .5f;
}

void Game::CountdownState::update(Game& game, std::chrono::milliseconds) {
	if (--countdown <= 0) {
		game.state = std::make_shared<PlayState>();
	}
}

void Game::CountdownState::render(Game& game, const Renderer::Ptr& renderer) {
	renderer->draw(renderer->getWhiteBrush(), game.mLeftScore);
	renderer->draw(renderer->getWhiteBrush(), game.mRightScore);
	renderer->draw(renderer->getWhiteBrush(), game.mBall);
	renderer->draw(renderer->getWhiteBrush(), game.mTopWall);
	renderer->draw(renderer->getWhiteBrush(), game.mBottomWall);
	renderer->draw(renderer->getWhiteBrush(), game.mLeftPaddle);
	renderer->draw(renderer->getWhiteBrush(), game.mRightPaddle);
}

void Game::PlayState::update(Game& game, std::chrono::milliseconds delta) {
	// Get the time (in milliseconds) we must consume during this simulation step.
	auto deltaMS = static_cast<float>(delta.count());

	// Apply the keyboard and gamepad input to paddle velocities.
	applyMovement(game.mLeftPaddle, player1Movement);
	applyMovement(game.mRightPaddle, player2Movement);

	do {
		// Perform collision detection to find out the first collision.
		const auto collision = game.detectCollision(deltaMS);
		if (collision.lhs == ObjectID::NONE && collision.rhs == ObjectID::NONE) {
			game.mLeftPaddle.position += game.mLeftPaddle.velocity * deltaMS;
			game.mRightPaddle.position += game.mRightPaddle.velocity * deltaMS;
			game.mBall.position += game.mBall.velocity * deltaMS;
			break;
		}

		// Consume simulation time and resolve collisions based on the first collision.
		const auto collisionMS = collision.time * deltaMS;
		deltaMS -= collisionMS;

		// Apply movement to dynamic entities.
		game.mBall.position += game.mBall.velocity * collisionMS;
		game.mLeftPaddle.position += game.mLeftPaddle.velocity * collisionMS;
		game.mRightPaddle.position += game.mRightPaddle.velocity * collisionMS;

		// Perform collision resolvement.
		game.resolveCollision(collision);
	} while (!game.mNewRound && game.player1Score < 10 && game.player2Score < 10);

	if (game.mNewRound) {
		game.state = std::make_shared<CountdownState>(game);
		game.mNewRound = false;
	}
}

void Game::PlayState::render(Game& game, const Renderer::Ptr& renderer) {
	renderer->draw(renderer->getWhiteBrush(), game.mLeftScore);
	renderer->draw(renderer->getWhiteBrush(), game.mRightScore);
	renderer->draw(renderer->getWhiteBrush(), game.mBall);
	renderer->draw(renderer->getWhiteBrush(), game.mTopWall);
	renderer->draw(renderer->getWhiteBrush(), game.mBottomWall);
	renderer->draw(renderer->getWhiteBrush(), game.mLeftPaddle);
	renderer->draw(renderer->getWhiteBrush(), game.mRightPaddle);
}

void Game::PlayState::onKeyDown(Game&, const winrt::Windows::UI::Core::KeyEventArgs& args) {
	switch (args.VirtualKey()) {
	case VirtualKey::Up:
		player2Movement = MoveDirection::UP;
		break;
	case VirtualKey::Down:
		player2Movement = MoveDirection::DOWN;
		break;
	case VirtualKey::W:
		player1Movement = MoveDirection::UP;
		break;
	case VirtualKey::S:
		player1Movement = MoveDirection::DOWN;
		break;
	}
}

void Game::PlayState::onKeyUp(Game&, const winrt::Windows::UI::Core::KeyEventArgs& args) {
	switch (args.VirtualKey()) {
	case VirtualKey::Up:
		player2Movement = (player2Movement == MoveDirection::UP ? MoveDirection::NONE : player2Movement);
		break;
	case VirtualKey::Down:
		player2Movement = (player2Movement == MoveDirection::DOWN ? MoveDirection::NONE : player2Movement);
		break;
	case VirtualKey::W:
		player1Movement = (player1Movement == MoveDirection::UP ? MoveDirection::NONE : player1Movement);
		break;
	case VirtualKey::S:
		player1Movement = (player1Movement == MoveDirection::DOWN ? MoveDirection::NONE : player1Movement);
		break;
	}
}

void Game::PlayState::onReadGamepad(Game&, int player, const winrt::Windows::Gaming::Input::GamepadReading& reading) {
	constexpr auto DeadZone = .25f;
	auto moveDirection = MoveDirection::NONE;
	if (reading.LeftThumbstickY > DeadZone) {
		moveDirection = MoveDirection::UP;
	} else if (reading.LeftThumbstickY < -DeadZone) {
		moveDirection = MoveDirection::DOWN;
	}
	if (player == 0) {
		player1Movement = moveDirection;
	} else {
		player2Movement = moveDirection;
	}
}

void Game::PlayState::applyMovement(Rectangle& rect, MoveDirection direction) {
	constexpr auto PaddleVelocity = .001f;
	switch (direction) {
	case MoveDirection::NONE:
		rect.velocity.y = 0.f;
		break;
	case MoveDirection::UP:
		rect.velocity.y = -PaddleVelocity;
		break;
	case MoveDirection::DOWN:
		rect.velocity.y = PaddleVelocity;
		break;
	}
}