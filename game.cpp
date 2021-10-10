#include "pch.hpp"
#include "game.hpp"

#include <random>

using namespace winrt::Windows::Gaming::Input;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::System;

Game::Game(const Audio& audio) {
	state = std::make_shared<Game::DialogState>(*this, L"Press X key or button to start a game");

	ball = std::make_shared<Rectangle>();
	ball->extent = { .0115f, .015f };
	ball->position = { .5f, .5f };

	topWall = std::make_shared<Rectangle>();
	topWall->extent = { .5f, .015f };
	topWall->position = { .5f, .015f };

	bottomWall = std::make_shared<Rectangle>();
	bottomWall->extent = topWall->extent;
	bottomWall->position = { .5f, .985f };

	leftPaddle = std::make_shared<Rectangle>();
	leftPaddle->extent = { .0125f, .075f };
	leftPaddle->position = { .05f, .5f };
	leftPaddle->velocity = { 0.f, 0.f };

	rightPaddle = std::make_shared<Rectangle>();
	rightPaddle->extent = leftPaddle->extent;
	rightPaddle->position = { .95f, .5f };
	rightPaddle->velocity = { 0.f, 0.f };

	leftScore = std::make_shared<Text>();
	leftScore->text = std::to_wstring(player1Score);
	leftScore->position = { .35f, .025f };
	leftScore->fontSize = .27f;

	rightScore = std::make_shared<Text>();
	rightScore->text = std::to_wstring(player2Score);
	rightScore->position = { .65f, .025f };
	rightScore->fontSize = .27f;

	leftGoal = std::make_shared<Rectangle>();
	leftGoal->extent = { .5f, .5f };
	leftGoal->position = { -.5f - ball->extent.x * 4.f, .5f };

	rightGoal = std::make_shared<Rectangle>();
	rightGoal->extent = leftGoal->extent;
	rightGoal->position = { 1.5f + ball->extent.x * 4.f, .5f };

	beepSound = audio.createSound(L"Assets/beep.wav");
}

auto Game::detectCollision(float deltaMS) const -> Collision {
	auto result = Collision{};
	detectCollision(deltaMS, ball, leftPaddle, result);
	detectCollision(deltaMS, ball, rightPaddle, result);
	detectCollision(deltaMS, ball, topWall, result);
	detectCollision(deltaMS, ball, bottomWall, result);
	detectCollision(deltaMS, ball, leftGoal, result);
	detectCollision(deltaMS, ball, rightGoal, result);
	detectCollision(deltaMS, leftPaddle, topWall, result);
	detectCollision(deltaMS, leftPaddle, bottomWall, result);
	detectCollision(deltaMS, rightPaddle, topWall, result);
	detectCollision(deltaMS, rightPaddle, bottomWall, result);
	return result;
}

void Game::detectCollision(float deltaMS, Rectangle::Ref r1, Rectangle::Ref r2, Collision& result) const {
	auto hit = detectCollision(deltaMS, r1, r2);
	if (hit.lhs && hit.time < result.time) {
		result.time = hit.time;
		result.lhs = r1;
		result.rhs = r2;
	}
}

auto Game::detectCollision(float deltaMS, Rectangle::Ref a, Rectangle::Ref b) const->Collision {
	auto collision = Collision{};

	const auto amin = a->position - a->extent;
	const auto amax = a->position + a->extent;
	const auto bmin = b->position - b->extent;
	const auto bmax = b->position + b->extent;

	// Exit early whether boxes initially collide.
	if (amin.x <= bmax.x && amax.x >= bmin.x && amin.y <= bmax.y && amax.y >= bmin.y) {
		collision.lhs = a;
		collision.rhs = b;
		collision.time = 0.f;
		return collision;
	}

	// We will use relative velocity where 'a' is treated as stationary.
	const auto v = (b->velocity - a->velocity) * deltaMS;

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
		collision.lhs = a;
		collision.rhs = b;
		collision.time = tmin * deltaMS;
	}
	return collision;
}

auto Game::resolveCollision(const Collision& collision) -> bool {
	constexpr auto BallVelocityMultiplier = 1.1f;
	constexpr auto Nudge = .001f;
	if (collision.lhs == ball) {
		if (collision.rhs == bottomWall) {
			ball->position.y = bottomWall->position.y - bottomWall->extent.y - ball->extent.y - Nudge;
			ball->velocity.y = -ball->velocity.y;
			beepSound.play();
		} else if (collision.rhs == topWall) {
			ball->position.y = topWall->position.y + topWall->extent.y + ball->extent.y + Nudge;
			ball->velocity.y = -ball->velocity.y;
			beepSound.play();
		} else if (collision.rhs == leftGoal) {
			player2Score++;
			if (player2Score >= 10) {
				state = std::make_shared<DialogState>(*this, L"Right player wins! Press X for rematch.");
			} else {
				rightScore->text = std::to_wstring(player2Score);
				state = std::make_shared<CountdownState>(*this);
			}
			return true;
		} else if (collision.rhs == rightGoal) {
			player1Score++;
			if (player1Score >= 10) {
				state = std::make_shared<DialogState>(*this, L"Left player wins! Press X for rematch.");
			} else {
				leftScore->text = std::to_wstring(player1Score);
				state = std::make_shared<CountdownState>(*this);
			}
			return true;
		} else if (collision.rhs == leftPaddle) {
			ball->position.x = leftPaddle->position.x + leftPaddle->extent.x + ball->extent.x + Nudge;
			ball->velocity.x = -ball->velocity.x;
			ball->velocity = ball->velocity * BallVelocityMultiplier;
			beepSound.play();
		} else if (collision.rhs == rightPaddle) {
			ball->position.x = rightPaddle->position.x - rightPaddle->extent.x - ball->extent.x - Nudge;
			ball->velocity.x = -ball->velocity.x;
			ball->velocity = ball->velocity * BallVelocityMultiplier;
			beepSound.play();
		}
	} else if (collision.lhs == leftPaddle) {
		if (collision.rhs == bottomWall) {
			leftPaddle->position.y = bottomWall->position.y - bottomWall->extent.y - leftPaddle->extent.y - Nudge;
		} else if (collision.rhs == topWall) {
			leftPaddle->position.y = topWall->position.y + topWall->extent.y + leftPaddle->extent.y + Nudge;
		}
		leftPaddle->velocity.y = 0.f;
	} else if (collision.lhs == rightPaddle) {
		if (collision.rhs == bottomWall) {
			rightPaddle->position.y = bottomWall->position.y - bottomWall->extent.y - rightPaddle->extent.y - Nudge;
		} else if (collision.rhs == topWall) {
			rightPaddle->position.y = topWall->position.y + topWall->extent.y + rightPaddle->extent.y + Nudge;
		}
		rightPaddle->velocity.y = 0.f;
	}
	return false;
}

Game::DialogState::DialogState(Game& game, const std::wstring& descriptionText) : State(game) {
	background = std::make_shared<Rectangle>();
	background->extent = { 0.375f, 0.40f };
	background->position = { .5f, .5f };

	foreground = std::make_shared<Rectangle>();
	foreground->extent = { 0.35f, 0.375f };
	foreground->position = { .5f, .5f };

	topic = std::make_shared<Text>();
	topic->text = L"UWP Pong";
	topic->position = { .5f, .3f };
	topic->fontSize = .1f;

	description = std::make_shared<Text>();
	description->text = descriptionText;
	description->position = { .5f, .6f };
	description->fontSize = .05f;
}

void Game::DialogState::render(const Renderer& renderer) {
	renderer.draw(renderer.getWhiteBrush(), background);
	renderer.draw(renderer.getBlackBrush(), foreground);
	renderer.draw(renderer.getWhiteBrush(), topic);
	renderer.draw(renderer.getWhiteBrush(), description);
}

void Game::DialogState::onKeyDown(const KeyEventArgs& args) {
	if (args.VirtualKey() == VirtualKey::X) {
		startGame();
	}
}

void Game::DialogState::onReadGamepad(int, const GamepadReading& reading) {
	if (GamepadButtons::X == (reading.Buttons & GamepadButtons::X)) {
		startGame();
	}
}

void Game::DialogState::startGame() {
	game.player1Score = 0;
	game.player2Score = 0;
	game.rightScore->text = std::to_wstring(game.player2Score);
	game.leftScore->text = std::to_wstring(game.player1Score);
	game.state = std::make_shared<CountdownState>(game);
}

Game::CountdownState::CountdownState(Game& game) : State(game) {
	game.ball->position = { .5f, .5f };
	game.ball->velocity = newRandomDirection();
	game.leftPaddle->position.y = .5f;
	game.rightPaddle->position.y = .5f;
}

void Game::CountdownState::update(std::chrono::milliseconds) {
	if (--countdown <= 0) {
		game.state = std::make_shared<PlayState>(game);
	}
}

void Game::CountdownState::render(const Renderer& renderer) {
	renderer.draw(renderer.getWhiteBrush(), game.leftScore);
	renderer.draw(renderer.getWhiteBrush(), game.rightScore);
	renderer.draw(renderer.getWhiteBrush(), game.ball);
	renderer.draw(renderer.getWhiteBrush(), game.topWall);
	renderer.draw(renderer.getWhiteBrush(), game.bottomWall);
	renderer.draw(renderer.getWhiteBrush(), game.leftPaddle);
	renderer.draw(renderer.getWhiteBrush(), game.rightPaddle);
}

auto Game::CountdownState::newRandomDirection() -> Vec2f {
	constexpr auto BallInitialVelocity = .0004f;
	static std::default_random_engine rng;
	static std::uniform_int_distribution<std::mt19937::result_type> dist(0, 3);
	switch (dist(rng)) {
	case 0: return Vec2f{ BallInitialVelocity, BallInitialVelocity };
	case 1: return Vec2f{ BallInitialVelocity, -BallInitialVelocity };
	case 2: return Vec2f{ -BallInitialVelocity, BallInitialVelocity };
	}
	return Vec2f{ -BallInitialVelocity, -BallInitialVelocity };
}

void Game::PlayState::update(std::chrono::milliseconds delta) {
	// Get the time (in milliseconds) we must consume during this simulation step.
	auto deltaMS = static_cast<float>(delta.count());

	// Apply the keyboard and gamepad input to paddle velocities.
	constexpr auto PaddleVelocity = .001f;
	game.leftPaddle->velocity.y = static_cast<float>(player1Movement) * PaddleVelocity;
	game.rightPaddle->velocity.y = static_cast<float>(player2Movement) * PaddleVelocity;

	auto endRound = false;
	do {
		// Perform collision detection to find out the first collision.
		const auto collision = game.detectCollision(deltaMS);
		if (!collision.lhs && !collision.rhs) {
			game.leftPaddle->position += game.leftPaddle->velocity * deltaMS;
			game.rightPaddle->position += game.rightPaddle->velocity * deltaMS;
			game.ball->position += game.ball->velocity * deltaMS;
			break;
		}

		// Consume simulation time and resolve collisions based on the first collision.
		deltaMS -= collision.time;

		// Apply movement to dynamic entities.
		game.ball->position += game.ball->velocity * collision.time;
		game.leftPaddle->position += game.leftPaddle->velocity * collision.time;
		game.rightPaddle->position += game.rightPaddle->velocity * collision.time;

		// Perform collision resolvement.
		endRound = game.resolveCollision(collision);
	} while (!endRound);
}

void Game::PlayState::render(const Renderer& renderer) {
	renderer.draw(renderer.getWhiteBrush(), game.leftScore);
	renderer.draw(renderer.getWhiteBrush(), game.rightScore);
	renderer.draw(renderer.getWhiteBrush(), game.ball);
	renderer.draw(renderer.getWhiteBrush(), game.topWall);
	renderer.draw(renderer.getWhiteBrush(), game.bottomWall);
	renderer.draw(renderer.getWhiteBrush(), game.leftPaddle);
	renderer.draw(renderer.getWhiteBrush(), game.rightPaddle);
}

void Game::PlayState::onKeyDown(const KeyEventArgs& args) {
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

void Game::PlayState::onKeyUp(const KeyEventArgs& args) {
	switch (args.VirtualKey()) {
	case VirtualKey::Up:
		player2Movement = std::max(MoveDirection::NONE, player2Movement);
		break;
	case VirtualKey::Down:
		player2Movement = std::min(MoveDirection::NONE, player2Movement);
		break;
	case VirtualKey::W:
		player1Movement = std::max(MoveDirection::NONE, player1Movement);
		break;
	case VirtualKey::S:
		player1Movement = std::min(MoveDirection::NONE, player1Movement);
		break;
	}
}

void Game::PlayState::onReadGamepad(int player, const GamepadReading& reading) {
	constexpr auto DeadZone = .25f;
	auto y = reading.LeftThumbstickY;
	switch (player) {
	case 0:
		player1Movement = (y > DeadZone ? MoveDirection::UP : y < -DeadZone ? MoveDirection::DOWN : MoveDirection::NONE);
		break;
	case 1:
		player2Movement = (y > DeadZone ? MoveDirection::UP : y < -DeadZone ? MoveDirection::DOWN : MoveDirection::NONE);
		break;
	}
}