#include "pch.h"
#include "aabb.h"
#include "scene.h"

#include <array>
#include <random>

// The center of the courtyard in y-axis.
static const auto CenterY = .5f;

// The center of the courtyard in x-axis.
static const auto CenterX = .5f;

// A constant presenting the center of the courtyard.
static const auto Center = Vec2f{ CenterX, CenterY };

// Build a randomly selected direction vector from 45, 135, 225 and 315 degrees.
inline auto NewRandomDirection() -> Vec2f {
	static std::default_random_engine rng;
	static std::uniform_int_distribution<std::mt19937::result_type> dist(0, 3);
	static const std::array<Vec2f, 4> dirs = {
		Vec2f(1.f, 1.f).normalized(),
		Vec2f(1.f, -1.f).normalized(),
		Vec2f(-1.f, 1.f).normalized(),
		Vec2f(-1.f, -1.f).normalized()
	};
	return dirs[dist(rng)];
}

// Calculate the dot product between the given vectors.
inline auto Dot(const Vec2f& v1, const Vec2f& v2) -> float {
	return v1.getX() * v2.getX() + v1.getY() * v2.getY();
}

// Reflect the incoming vector v with the given surface normal n.
inline auto ReflectVector(const Vec2f& v, const Vec2f& n) -> Vec2f {
	return v - n * 2.f * Dot(v, n);
}

Scene::Scene() {
	mBall.setSize({ .023f, .03f });
	mBall.setPosition(Center);

	mUpperWall.setSize({ 1.f, .03f });
	mUpperWall.setPosition({ CenterX, .015f });

	mLowerWall.setSize({ 1.f, .03f });
	mLowerWall.setPosition({ CenterX, .985f });

	mLeftPaddle.setSize({ .025f, .15f });
	mLeftPaddle.setPosition({ .05f, CenterY });

	mRightPaddle.setSize({ .025f, .15f });
	mRightPaddle.setPosition({ .95f, CenterY });

	mLeftScore.setText(std::to_wstring(ctx.P1Score));
	mLeftScore.setPosition({ .35f, .025f });

	mRightScore.setText(std::to_wstring(ctx.P2Score));
	mRightScore.setPosition({ .65f, .025f });

	mLeftGoal.setSize({ 1.f, 1.f });
	mLeftGoal.setPosition({ -.5f - mBall.getSize().getX() * 2.f, CenterY });

	mRightGoal.setSize({ 1.f, 1.f });
	mRightGoal.setPosition({ 1.5f + mBall.getSize().getX() * 2.f, CenterY });

	resetGame();
}

auto Scene::broadCD(const Vec2f& pL, const Vec2f& pR, const Vec2f& pB) const -> std::vector<Candidate> {
	// Build bounding boxes for dynamic entities based on their current and ideal new positions.
	auto bbL = mLeftPaddle.getAABB() + AABB(pL, mLeftPaddle.getExtent());
	auto bbR = mRightPaddle.getAABB() + AABB(pR, mRightPaddle.getExtent());
	auto bbB = mBall.getAABB() + AABB(pB, mBall.getExtent());

	// Go through and pick all possible collision candidates.
	std::vector<Candidate> candidates;
	if (mBall.getVelocity().getX() < 0.f && bbB.collides(bbL)) {
		candidates.push_back({ CandidateType::BALL, CandidateType::LPADDLE });
	} else if (mBall.getVelocity().getX() > 0.f && bbB.collides(bbR)) {
		candidates.push_back({ CandidateType::BALL, CandidateType::RPADDLE });
	}
	if (bbB.collides(mUpperWall.getAABB())) {
		candidates.push_back({ CandidateType::BALL, CandidateType::TWALL });
	} else if (bbB.collides(mLowerWall.getAABB())) {
		candidates.push_back({ CandidateType::BALL, CandidateType::BWALL });
	}
	if (bbB.collides(mLeftGoal.getAABB())) {
		candidates.push_back({ CandidateType::BALL, CandidateType::LGOAL });
	} else if (bbB.collides(mRightGoal.getAABB())) {
		candidates.push_back({ CandidateType::BALL, CandidateType::RGOAL });
	}
	if (bbR.collides(mUpperWall.getAABB())) {
		candidates.push_back({ CandidateType::RPADDLE, CandidateType::TWALL });
	} else if (bbR.collides(mLowerWall.getAABB())) {
		candidates.push_back({ CandidateType::RPADDLE, CandidateType::BWALL });
	}
	if (bbL.collides(mUpperWall.getAABB())) {
		candidates.push_back({ CandidateType::LPADDLE, CandidateType::TWALL });
	} else if (bbL.collides(mLowerWall.getAABB())) {
		candidates.push_back({ CandidateType::LPADDLE, CandidateType::BWALL });
	}
	return candidates;
}

auto Scene::narrowCD(const std::vector<Candidate>& candidates, const Vec2f& vL, const Vec2f& vR, float deltaMS) const -> NarrowCDResult {
	const auto& ballAABB = mBall.getAABB();

	auto result = NarrowCDResult{};
	result.hasHit = false;
	result.hitTime = FLT_MAX;
	for (auto& candidate : candidates) {
		AABB::Sweep hit = {};
		Rectangle lhs;
		switch (candidate.lhs) {
		case CandidateType::BALL:
			switch (candidate.rhs) {
			case CandidateType::LPADDLE:
				hit = AABB::sweep(ballAABB, mLeftPaddle.getAABB(), mBall.getVelocity() * deltaMS, vL);
				break;
			case CandidateType::RPADDLE:
				hit = AABB::sweep(ballAABB, mRightPaddle.getAABB(), mBall.getVelocity() * deltaMS, vR);
				break;
			case CandidateType::TWALL:
				hit = AABB::sweep(ballAABB, mUpperWall.getAABB(), mBall.getVelocity() * deltaMS, { 0.f, 0.f });
				break;
			case CandidateType::BWALL:
				hit = AABB::sweep(ballAABB, mLowerWall.getAABB(), mBall.getVelocity() * deltaMS, { 0.f, 0.f });
				break;
			case CandidateType::LGOAL:
				hit = AABB::sweep(ballAABB, mLeftGoal.getAABB(), mBall.getVelocity() * deltaMS, { 0.f, 0.f });
				break;
			case CandidateType::RGOAL:
				hit = AABB::sweep(ballAABB, mRightGoal.getAABB(), mBall.getVelocity() * deltaMS, { 0.f, 0.f });
				break;
			}
			break;
		case CandidateType::LPADDLE:
			switch (candidate.rhs) {
			case CandidateType::TWALL:
				if (vL.getY() < 0.f) {
					hit = AABB::sweep(mLeftPaddle.getAABB(), mUpperWall.getAABB(), vL * deltaMS, { 0.f, 0.f });
				}
				break;
			case CandidateType::BWALL:
				if (vL.getY() > 0.f) {
					hit = AABB::sweep(mLeftPaddle.getAABB(), mLowerWall.getAABB(), vL * deltaMS, { 0.f, 0.f });
				}
				break;
			}
			break;
		case CandidateType::RPADDLE:
			switch (candidate.rhs) {
			case CandidateType::TWALL:
				if (vR.getY() < 0.f) {
					hit = AABB::sweep(mRightPaddle.getAABB(), mUpperWall.getAABB(), vR * deltaMS, { 0.f,0.f });
				}
				break;
			case CandidateType::BWALL:
				if (vR.getY() > 0.f) {
					hit = AABB::sweep(mRightPaddle.getAABB(), mLowerWall.getAABB(), vR * deltaMS, { 0.f,0.f });
				}
				break;
			}
			break;
		}
		if (hit.hit.collided && hit.time < result.hitTime) {
			result.hitTime = hit.time;
			result.hasHit = true;
			result.candidate = candidate;
			result.normal = hit.hit.normal;
		}
	}
	return result;
}

void Scene::update(std::chrono::milliseconds delta) {
	// Get the time (in milliseconds) we must consume during this simulation step and also lock
	// the current velocities of the paddles to prevent input to change velocity on-the-fly.
	auto deltaMS = static_cast<float>(delta.count());
	Vec2f vL = mLeftPaddle.getVelocity();
	Vec2f vR = mRightPaddle.getVelocity();

	// TODO A temporary solution which should be handled in a more elegant way.
	auto mustResetGame = false;
	do {
		// Calculate the new ideal positions for the dynamic entities.
		const auto pL = mLeftPaddle.getPosition() + vL * deltaMS;
		const auto pR = mRightPaddle.getPosition() + vR * deltaMS;
		const auto pB = mBall.getPosition() + mBall.getVelocity() * deltaMS;

		// Find collision cadidates with the broad phase of the collision detection.
		const auto collisionCandidates = broadCD(pL, pR, pB);
		if (collisionCandidates.empty()) {
			mLeftPaddle.setPosition(pL);
			mRightPaddle.setPosition(pR);
			mBall.setPosition(pB);
			break;
		}

		// Use the narrow phase of the collision detection to find out the first collision.
		const auto collision = narrowCD(collisionCandidates, vL, vR, deltaMS);
		if (!collision.hasHit) {
			mLeftPaddle.setPosition(pL);
			mRightPaddle.setPosition(pR);
			mBall.setPosition(pB);
			break;
		}

		// Consume simulation time and resolve collisions based on the first collision.
		const auto collisionMS = collision.hitTime * deltaMS;
		deltaMS -= collisionMS;

		switch (collision.candidate.lhs) {
		case CandidateType::BALL:
			mLeftPaddle.setPosition(mLeftPaddle.getPosition() + vL * collisionMS);
			mRightPaddle.setPosition(mRightPaddle.getPosition() + vR * collisionMS);
			switch (collision.candidate.rhs) {
			case CandidateType::BWALL:
			case CandidateType::TWALL:
				mBall.setPosition(mBall.getPosition() + mBall.getVelocity() * collisionMS);
				mBall.setVelocity(ReflectVector(mBall.getVelocity(), collision.normal));
				break;
			case CandidateType::LGOAL:
				ctx.P2Score = (ctx.P2Score + 1) % 10;
				mRightScore.setText(std::to_wstring(ctx.P2Score));
				// TODO implement end of game logics?
				mustResetGame = true;
				break;
			case CandidateType::RGOAL:
				ctx.P1Score = (ctx.P1Score + 1) % 10;
				mLeftScore.setText(std::to_wstring(ctx.P1Score));
				// TODO implement end of game logics?
				mustResetGame = true;
				break;
			case CandidateType::LPADDLE:
				mBall.setPosition(mBall.getPosition() + mBall.getVelocity() * collisionMS);
				mBall.setVelocity(ReflectVector(mBall.getVelocity(), collision.normal));
				mBall.setPosition(mBall.getPosition() + mBall.getVelocity());
				break;
			case CandidateType::RPADDLE:
				mBall.setPosition(mBall.getPosition() + mBall.getVelocity() * collisionMS);
				mBall.setVelocity(ReflectVector(mBall.getVelocity(), collision.normal));
				mBall.setPosition(mBall.getPosition() + mBall.getVelocity());
				break;
			}
			break;
		case CandidateType::LPADDLE:
			mRightPaddle.setPosition(mRightPaddle.getPosition() + vR * collisionMS);
			mBall.setPosition(mBall.getPosition() + mBall.getVelocity() * collisionMS);
			switch (collision.candidate.rhs) {
			case CandidateType::BWALL:
				mLeftPaddle.setPositionY(mLowerWall.getAABB().getMinY() - mLeftPaddle.getExtent().getY() - .001f);
				vL.setY(0.f);
				break;
			case CandidateType::TWALL:
				mLeftPaddle.setPositionY(mUpperWall.getAABB().getMaxY() + mLeftPaddle.getExtent().getY() + .001f);
				vL.setY(0.f);
				break;
			}
			break;
		case CandidateType::RPADDLE:
			mLeftPaddle.setPosition(mLeftPaddle.getPosition() + vL * collisionMS);
			mBall.setPosition(mBall.getPosition() + mBall.getVelocity() * collisionMS);
			switch (collision.candidate.rhs) {
			case CandidateType::BWALL:
				mRightPaddle.setPositionY(mLowerWall.getAABB().getMinY() - mRightPaddle.getExtent().getY() - .001f);
				vR.setY(0.f);
				break;
			case CandidateType::TWALL:
				mRightPaddle.setPositionY(mUpperWall.getAABB().getMaxY() + mRightPaddle.getExtent().getY() + .001f);
				vR.setY(0.f);
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
	mLeftScore.render(renderer);
	mRightScore.render(renderer);
	mBall.render(renderer);
	mUpperWall.render(renderer);
	mLowerWall.render(renderer);
	mLeftPaddle.render(renderer);
	mRightPaddle.render(renderer);
}

void Scene::resetGame() {
	mBall.setPosition(Center);
	mBall.setVelocity(NewRandomDirection() * .0005f);
	mLeftPaddle.setPositionY(CenterY);
	mRightPaddle.setPositionY(CenterY);
}