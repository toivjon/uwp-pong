#include "pch.h"
#include "aabb.h"
#include "scene.h"

#include <array>
#include <random>

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
static const auto BallInitialVelocity = .0005f;

// The amount of velocity to increment during each ball-paddle collision.
static const auto BallVelocityIncrement = .0001f;

// The amount of additional nudge to add to collision response to separate collded objects.
static const auto Nudge = .001f;

// The velocity of player controlled paddles.
static const auto PaddleVelocity = .001f;

// Build a randomly selected direction vector from 45, 135, 225 and 315 degrees.
inline auto NewRandomDirection() -> Vec2f {
	static std::default_random_engine rng;
	static std::uniform_int_distribution<std::mt19937::result_type> dist(0, 3);
	static const std::array<Vec2f, 4> dirs = {
		Vec2f{1.f, 1.f}.normalized(),
		Vec2f{1.f, -1.f}.normalized(),
		Vec2f{-1.f, 1.f}.normalized(),
		Vec2f{-1.f, -1.f}.normalized()
	};
	return dirs[dist(rng)];
}

// Build a new AABB from a Rectangle instance.
inline auto RectangleToAABB(const Rectangle& rect) -> AABB {
	return AABB(rect.position, rect.size / 2.f);
}

Scene::Scene(const Renderer::Ptr& renderer) : mShowWelcomeDialog(true) {
	mDialogBackground.size = { 0.75f, 0.80f };
	mDialogBackground.position = { Center };
	mDialogBackground.brush = renderer->getWhiteBrush();

	mDialogForeground.size = { 0.70f, 0.75f };
	mDialogForeground.position = { Center };
	mDialogForeground.brush = renderer->getBlackBrush();

	mDialogTopic.brush = renderer->getWhiteBrush();
	mDialogTopic.text = L"UWP Pong";
	mDialogTopic.position = { CenterX, .3f };
	mDialogTopic.fontSize = .1f;

	mDialogDescription.brush = renderer->getWhiteBrush();
	mDialogDescription.text = L"Press any key or button to start a game";
	mDialogDescription.position = { CenterX, .6f };
	mDialogDescription.fontSize = .05f;

	mBall.size = { .023f, .03f };
	mBall.position = Center;
	mBall.brush = renderer->getWhiteBrush();

	mUpperWall.size = { 1.f, .03f };
	mUpperWall.position = { CenterX, .015f };
	mUpperWall.brush = renderer->getWhiteBrush();

	mLowerWall.size = { 1.f, .03f };
	mLowerWall.position = { CenterX, .985f };
	mLowerWall.brush = renderer->getWhiteBrush();

	mLeftPaddle.size = { .025f, .15f };
	mLeftPaddle.position = { .05f, CenterY };
	mLeftPaddle.brush = renderer->getWhiteBrush();
	mLeftPaddle.velocity = { 0.f, 0.f };

	mRightPaddle.size = { .025f, .15f };
	mRightPaddle.position = { .95f, CenterY };
	mRightPaddle.brush = renderer->getWhiteBrush();
	mRightPaddle.velocity = { 0.f, 0.f };

	mLeftScore.text = std::to_wstring(ctx.P1Score);
	mLeftScore.position = { .35f, .025f };
	mLeftScore.brush = renderer->getWhiteBrush();
	mLeftScore.fontSize = .27f;

	mRightScore.text = std::to_wstring(ctx.P2Score);
	mRightScore.position = { .65f, .025f };
	mRightScore.brush = renderer->getWhiteBrush();
	mRightScore.fontSize = .27f;

	mLeftGoal.size = { 1.f, 1.f };
	mLeftGoal.position = { -.5f - mBall.size.x * 2.f, CenterY };
	mLeftGoal.brush = renderer->getWhiteBrush();

	mRightGoal.size = { 1.f, 1.f };
	mRightGoal.position = { 1.5f + mBall.size.x * 2.f, CenterY };
	mRightGoal.brush = renderer->getWhiteBrush();

	resetGame();
}

auto Scene::broadCD(const Vec2f& pL, const Vec2f& pR, const Vec2f& pB) const -> std::vector<Candidate> {
	// Build bounding boxes for dynamic entities based on their current and ideal new positions.
	auto bbL = RectangleToAABB(mLeftPaddle) + AABB(pL, mLeftPaddle.size / 2.f);
	auto bbR = RectangleToAABB(mRightPaddle) + AABB(pR, mRightPaddle.size / 2.f);
	auto bbBX = RectangleToAABB(mBall);
	auto bbB = RectangleToAABB(mBall) + AABB(pB, mBall.size / 2.f);

	// Go through and pick all possible collision candidates.
	std::vector<Candidate> candidates;
	if (mBall.velocity.x < 0.f && AABB::intersect(bbB, bbL)) {
		candidates.push_back({ CandidateType::BALL, CandidateType::LPADDLE });
	} else if (mBall.velocity.x > 0.f && AABB::intersect(bbB, bbR)) {
		candidates.push_back({ CandidateType::BALL, CandidateType::RPADDLE });
	}
	if (AABB::intersect(bbB, RectangleToAABB(mUpperWall))) {
		candidates.push_back({ CandidateType::BALL, CandidateType::TWALL });
	} else if (AABB::intersect(bbB, RectangleToAABB(mLowerWall))) {
		candidates.push_back({ CandidateType::BALL, CandidateType::BWALL });
	}
	if (AABB::intersect(bbB, RectangleToAABB(mLeftGoal))) {
		candidates.push_back({ CandidateType::BALL, CandidateType::LGOAL });
	} else if (AABB::intersect(bbB, RectangleToAABB(mRightGoal))) {
		candidates.push_back({ CandidateType::BALL, CandidateType::RGOAL });
	}
	if (AABB::intersect(bbR, RectangleToAABB(mUpperWall))) {
		candidates.push_back({ CandidateType::RPADDLE, CandidateType::TWALL });
	} else if (AABB::intersect(bbR, RectangleToAABB(mLowerWall))) {
		candidates.push_back({ CandidateType::RPADDLE, CandidateType::BWALL });
	}
	if (AABB::intersect(bbL, RectangleToAABB(mUpperWall))) {
		candidates.push_back({ CandidateType::LPADDLE, CandidateType::TWALL });
	} else if (AABB::intersect(bbL, RectangleToAABB(mLowerWall))) {
		candidates.push_back({ CandidateType::LPADDLE, CandidateType::BWALL });
	}
	return candidates;
}

auto Scene::narrowCD(const std::vector<Candidate>& candidates, const Vec2f& vL, const Vec2f& vR, float deltaMS) const -> NarrowCDResult {
	const auto& ballAABB = RectangleToAABB(mBall);

	auto result = NarrowCDResult{};
	result.hasHit = false;
	result.hitTime = FLT_MAX;
	for (auto& candidate : candidates) {
		AABB::Intersection hit = {};
		switch (candidate.lhs) {
		case CandidateType::BALL:
			switch (candidate.rhs) {
			case CandidateType::LPADDLE:
				hit = AABB::intersect(ballAABB, RectangleToAABB(mLeftPaddle), mBall.velocity * deltaMS, vL);
				break;
			case CandidateType::RPADDLE:
				hit = AABB::intersect(ballAABB, RectangleToAABB(mRightPaddle), mBall.velocity * deltaMS, vR);
				break;
			case CandidateType::TWALL:
				hit = AABB::intersect(ballAABB, RectangleToAABB(mUpperWall), mBall.velocity * deltaMS, { 0.f, 0.f });
				break;
			case CandidateType::BWALL:
				hit = AABB::intersect(ballAABB, RectangleToAABB(mLowerWall), mBall.velocity * deltaMS, { 0.f, 0.f });
				break;
			case CandidateType::LGOAL:
				hit = AABB::intersect(ballAABB, RectangleToAABB(mLeftGoal), mBall.velocity * deltaMS, { 0.f, 0.f });
				break;
			case CandidateType::RGOAL:
				hit = AABB::intersect(ballAABB, RectangleToAABB(mRightGoal), mBall.velocity * deltaMS, { 0.f, 0.f });
				break;
			}
			break;
		case CandidateType::LPADDLE:
			switch (candidate.rhs) {
			case CandidateType::TWALL:
				if (vL.y < 0.f) {
					hit = AABB::intersect(RectangleToAABB(mLeftPaddle), RectangleToAABB(mUpperWall), vL * deltaMS, { 0.f, 0.f });
				}
				break;
			case CandidateType::BWALL:
				if (vL.y > 0.f) {
					hit = AABB::intersect(RectangleToAABB(mLeftPaddle), RectangleToAABB(mLowerWall), vL * deltaMS, { 0.f, 0.f });
				}
				break;
			}
			break;
		case CandidateType::RPADDLE:
			switch (candidate.rhs) {
			case CandidateType::TWALL:
				if (vR.y < 0.f) {
					hit = AABB::intersect(RectangleToAABB(mRightPaddle), RectangleToAABB(mUpperWall), vR * deltaMS, { 0.f,0.f });
				}
				break;
			case CandidateType::BWALL:
				if (vR.y > 0.f) {
					hit = AABB::intersect(RectangleToAABB(mRightPaddle), RectangleToAABB(mLowerWall), vR * deltaMS, { 0.f,0.f });
				}
				break;
			}
			break;
		}
		if (hit.collides && hit.time < result.hitTime) {
			result.hitTime = hit.time;
			result.hasHit = true;
			result.candidate = candidate;
		}
	}
	return result;
}

void Scene::update(std::chrono::milliseconds delta) {
	// Skip the update whether the game has been just launched or the game has ended.
	if (mShowWelcomeDialog || mShowEndgameDialog) {
		return;
	}

	// Skip the update whether the countdown is still in progress.
	if (ctx.Countdown > 0) {
		ctx.Countdown--;
		return;
	}

	// Get the time (in milliseconds) we must consume during this simulation step and also lock
	// the current velocities of the paddles to prevent input to change velocity on-the-fly.
	auto deltaMS = static_cast<float>(delta.count());
	Vec2f vL = mLeftPaddle.velocity;
	Vec2f vR = mRightPaddle.velocity;

	// TODO A temporary solution which should be handled in a more elegant way.
	auto mustResetGame = false;
	do {
		// Calculate the new ideal positions for the dynamic entities.
		const auto pL = mLeftPaddle.position + vL * deltaMS;
		const auto pR = mRightPaddle.position + vR * deltaMS;
		const auto pB = mBall.position + mBall.velocity * deltaMS;

		// Find collision cadidates with the broad phase of the collision detection.
		const auto collisionCandidates = broadCD(pL, pR, pB);
		if (collisionCandidates.empty()) {
			mLeftPaddle.position = pL;
			mRightPaddle.position = pR;
			mBall.position = pB;
			break;
		}

		// Use the narrow phase of the collision detection to find out the first collision.
		const auto collision = narrowCD(collisionCandidates, vL, vR, deltaMS);
		if (!collision.hasHit) {
			mLeftPaddle.position = pL;
			mRightPaddle.position = pR;
			mBall.position = pB;
			break;
		}

		// Consume simulation time and resolve collisions based on the first collision.
		const auto collisionMS = collision.hitTime * deltaMS;
		deltaMS -= collisionMS;

		switch (collision.candidate.lhs) {
		case CandidateType::BALL:
			mLeftPaddle.position += vL * collisionMS;
			mRightPaddle.position += vR * collisionMS;
			switch (collision.candidate.rhs) {
			case CandidateType::BWALL:
				mBall.position += mBall.velocity * collisionMS;
				mBall.position.y = RectangleToAABB(mLowerWall).min.y - (mBall.size / 2.f).y - Nudge;
				mBall.velocity.y = -mBall.velocity.y;
				break;
			case CandidateType::TWALL:
				mBall.position += mBall.velocity * collisionMS;
				mBall.position.y = RectangleToAABB(mUpperWall).max.y + (mBall.size / 2.f).y + Nudge;
				mBall.velocity.y = -mBall.velocity.y;
				break;
			case CandidateType::LGOAL:
				ctx.P2Score++;
				if (ctx.P2Score >= 10) {
					mShowEndgameDialog = true;
					mDialogTopic.text = L"Right player wins!";
				} else {
					mRightScore.text = std::to_wstring(ctx.P2Score);
				}
				mustResetGame = true;
				break;
			case CandidateType::RGOAL:
				ctx.P1Score++;
				if (ctx.P1Score >= 10) {
					mShowEndgameDialog = true;
					mDialogTopic.text = L"Left player wins!";
				} else {
					mLeftScore.text = std::to_wstring(ctx.P1Score);
				}
				mustResetGame = true;
				break;
			case CandidateType::LPADDLE: {
				mBall.position += mBall.velocity * collisionMS;
				mBall.velocity.x = -mBall.velocity.x;
				mBall.velocity = mBall.velocity.normalized() * (mBall.velocity.length() + BallVelocityIncrement);
				break;
			}
			case CandidateType::RPADDLE:
				mBall.position += mBall.velocity * collisionMS;
				mBall.velocity.x = -mBall.velocity.x;
				mBall.velocity = mBall.velocity.normalized() * (mBall.velocity.length() + BallVelocityIncrement);
				break;
			}
			break;
		case CandidateType::LPADDLE:
			mRightPaddle.position += vR * collisionMS;
			mBall.position += mBall.velocity * collisionMS;
			switch (collision.candidate.rhs) {
			case CandidateType::BWALL:
				mLeftPaddle.position.y = RectangleToAABB(mLowerWall).min.y - (mLeftPaddle.size / 2.f).y - Nudge;
				vL.y = 0.f;
				break;
			case CandidateType::TWALL:
				mLeftPaddle.position.y = RectangleToAABB(mUpperWall).max.y + (mLeftPaddle.size / 2.f).y + Nudge;
				vL.y = 0.f;
				break;
			}
			break;
		case CandidateType::RPADDLE:
			mLeftPaddle.position += vL * collisionMS;
			mBall.position += mBall.velocity * collisionMS;
			switch (collision.candidate.rhs) {
			case CandidateType::BWALL:
				mRightPaddle.position.y = RectangleToAABB(mLowerWall).min.y - (mRightPaddle.size / 2.f).y - Nudge;
				vR.y = 0.f;
				break;
			case CandidateType::TWALL:
				mRightPaddle.position.y = RectangleToAABB(mUpperWall).max.y + (mRightPaddle.size / 2.f).y + Nudge;
				vR.y = 0.f;
				break;
			}
		}
		if (mustResetGame) {
			break;
		}
	} while (true);

	if (mustResetGame) {
		resetGame();
	}
}

void Scene::render(const Renderer::Ptr& renderer) const {
	// Base game entities are always shown.
	renderer->draw(mLeftScore);
	renderer->draw(mRightScore);
	renderer->draw(mBall);
	renderer->draw(mUpperWall);
	renderer->draw(mLowerWall);
	renderer->draw(mLeftPaddle);
	renderer->draw(mRightPaddle);

	// Dialog stuff is shown only on game enter or end game.
	if (mShowWelcomeDialog || mShowEndgameDialog) {
		renderer->draw(mDialogBackground);
		renderer->draw(mDialogForeground);
		renderer->draw(mDialogTopic);
		renderer->draw(mDialogDescription);
	}
}

// TODO Just store the paddle movement request. Apply it in the update to prevent stutter.
void Scene::onKeyDown(const KeyEventArgs& args) {
	if (mShowEndgameDialog || mShowWelcomeDialog) {
		mShowEndgameDialog = false;
		mShowWelcomeDialog = false;
		ctx.P1Score = 0;
		ctx.P2Score = 0;
		mRightScore.text = std::to_wstring(ctx.P2Score);
		mLeftScore.text = std::to_wstring(ctx.P1Score);
	} else {
		switch (args.VirtualKey()) {
		case VirtualKey::Up:
			mRightPaddle.velocity.y = -PaddleVelocity;
			break;
		case VirtualKey::Down:
			mRightPaddle.velocity.y = PaddleVelocity;
			break;
		case VirtualKey::W:
			mLeftPaddle.velocity.y = -PaddleVelocity;
			break;
		case VirtualKey::S:
			mLeftPaddle.velocity.y = PaddleVelocity;
			break;
		}
	}
}

// TODO Just store the paddle movement request. Apply it in the update to prevent stutter.
void Scene::onKeyUp(const KeyEventArgs& args) {
	switch (args.VirtualKey()) {
	case VirtualKey::Up:
		if (mRightPaddle.velocity.y < 0.f) {
			mRightPaddle.velocity.y = 0.f;
		}
		break;
	case VirtualKey::Down:
		if (mRightPaddle.velocity.y > 0.f) {
			mRightPaddle.velocity.y = 0.f;
		}
		break;
	case VirtualKey::W:
		if (mLeftPaddle.velocity.y < 0.f) {
			mLeftPaddle.velocity.y = 0.f;
		}
		break;
	case VirtualKey::S:
		if (mLeftPaddle.velocity.y > 0.f) {
			mLeftPaddle.velocity.y = 0.f;
		}
		break;
	}
}

void Scene::resetGame() {
	mBall.position = Center;
	mBall.velocity = NewRandomDirection() * BallInitialVelocity;
	mLeftPaddle.position.y = CenterY;
	mRightPaddle.position.y = CenterY;
	ctx.Countdown = CountdownTicks;
}