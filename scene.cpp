#include "pch.h"
#include "aabb.h"
#include "scene.h"

// A constant presenting the center of the courtyard.
static const auto Center = Vec2f{ .5f, .5f };

Scene::Scene() {
	mBall.setSize({ .023f, .03f });
	mBall.setPosition(Center);
	mBall.setVelocity(Vec2f({ 1.f, 1.f }).normalized() * .0005f);

	mUpperWall.setSize({ 1.f, .03f });
	mUpperWall.setPosition({ .5f, .015f });

	mLowerWall.setSize({ 1.f, .03f });
	mLowerWall.setPosition({ .5f, .985f });

	mLeftPaddle.setSize({ .025f, .15f });
	mLeftPaddle.setPosition({ .05f, .2f });

	mRightPaddle.setSize({ .025f, .15f });
	mRightPaddle.setPosition({ .95f, .8f });

	mLeftScore.setText(L"0");
	mLeftScore.setPosition({ .35f, .025f });

	mRightScore.setText(L"0");
	mRightScore.setPosition({ .65f, .025f });

	mLeftGoal.setSize({ 1.f, 1.f });
	mLeftGoal.setPosition({ -.5f - mBall.getSize().getX() * 2.f, .5f });

	mRightGoal.setSize({ 1.f, 1.f });
	mRightGoal.setPosition({ 1.5f + mBall.getSize().getX() * 2.f, .5f });
}

auto Scene::broadCD(const Vec2f& pL, const Vec2f& pR, const Vec2f& pB) const -> std::vector<Candidate> {
	// Build bounding boxes for dynamic entities based on their current and ideal new positions.
	auto bbL = mLeftPaddle.getAABB() + AABB(pL, mLeftPaddle.getExtent());
	auto bbR = mRightPaddle.getAABB() + AABB(pR, mRightPaddle.getExtent());
	auto bbB = mBall.getAABB() + AABB(pB, mBall.getExtent());

	// Go through and pick all possible collision candidates.
	std::vector<Candidate> candidates;
	if (bbB.collides(bbL)) {
		candidates.push_back({ CandidateType::BALL, CandidateType::LPADDLE });
	} else if (bbB.collides(bbR)) {
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

auto Scene::narrowCD(const std::vector<Candidate>& candidates, const Vec2f& vL, const Vec2f& vR) const -> NarrowCDResult {
	auto result = NarrowCDResult{};
	result.hasHit = false;
	result.hitTime = FLT_MAX;
	for (auto& candidate : candidates) {
		AABB::Intersection hit = {};
		Rectangle lhs;
		switch (candidate.lhs) {
		case CandidateType::BALL:
			switch (candidate.rhs) {
			case CandidateType::LPADDLE:
				hit = AABB::intersect(mBall.getAABB(), mLeftPaddle.getAABB(), mBall.getVelocity(), vL);
				break;
			case CandidateType::RPADDLE:
				hit = AABB::intersect(mBall.getAABB(), mRightPaddle.getAABB(), mBall.getVelocity(), vR);
				break;
			case CandidateType::TWALL:
				hit = AABB::intersect(mBall.getAABB(), mUpperWall.getAABB(), mBall.getVelocity(), { 0.f, 0.f });
				break;
			case CandidateType::BWALL:
				hit = AABB::intersect(mBall.getAABB(), mLowerWall.getAABB(), mBall.getVelocity(), { 0.f, 0.f });
				break;
			case CandidateType::LGOAL:
				hit = AABB::intersect(mBall.getAABB(), mLeftGoal.getAABB(), mBall.getVelocity(), { 0.f, 0.f });
				break;
			case CandidateType::RGOAL:
				hit = AABB::intersect(mBall.getAABB(), mRightGoal.getAABB(), mBall.getVelocity(), { 0.f, 0.f });
				break;
			}
			break;
		case CandidateType::LPADDLE:
			switch (candidate.rhs) {
			case CandidateType::TWALL:
				if (vL.getY() < 0.f) {
					hit = AABB::intersect(mLeftPaddle.getAABB(), mUpperWall.getAABB(), vL, { 0.f, 0.f });
				}
				break;
			case CandidateType::BWALL:
				if (vL.getY() > 0.f) {
					hit = AABB::intersect(mLeftPaddle.getAABB(), mLowerWall.getAABB(), vL, { 0.f, 0.f });
				}
				break;
			}
			break;
		case CandidateType::RPADDLE:
			switch (candidate.rhs) {
			case CandidateType::TWALL:
				if (vR.getY() < 0.f) {
					hit = AABB::intersect(mRightPaddle.getAABB(), mUpperWall.getAABB(), vR, { 0.f,0.f });
				}
				break;
			case CandidateType::BWALL:
				if (vR.getY() > 0.f) {
					hit = AABB::intersect(mRightPaddle.getAABB(), mLowerWall.getAABB(), vR, { 0.f,0.f });
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
		const auto collision = narrowCD(collisionCandidates, vL, vR);
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
				mBall.setVelocity({ mBall.getVelocity().getX(), -mBall.getVelocity().getY() });
				break;
			case CandidateType::LGOAL:
			case CandidateType::RGOAL:
				// TODO implement scoring logics
				mustResetGame = true;
				break;
			case CandidateType::LPADDLE:
			case CandidateType::RPADDLE:
				mBall.setPosition(mBall.getPosition() + mBall.getVelocity() * collisionMS);
				mBall.setVelocity({ -mBall.getVelocity().getX(), mBall.getVelocity().getY() });
				break;
			}
			break;
		case CandidateType::LPADDLE:
			mRightPaddle.setPosition(mRightPaddle.getPosition() + vR * collisionMS);
			mBall.setPosition(mBall.getPosition() + mBall.getVelocity() * collisionMS);
			switch (collision.candidate.rhs) {
			case CandidateType::BWALL:
				mLeftPaddle.setPosition({ mLeftPaddle.getPosition().getX(), mLowerWall.getAABB().getMinY() - mLeftPaddle.getExtent().getY() - .001f });
				vL.setY(0.f);
				break;
			case CandidateType::TWALL:
				mLeftPaddle.setPosition({ mLeftPaddle.getPosition().getX(), mUpperWall.getAABB().getMaxY() + mLeftPaddle.getExtent().getY() + .001f });
				vL.setY(0.f);
				break;
			}
			break;
		case CandidateType::RPADDLE:
			mLeftPaddle.setPosition(mLeftPaddle.getPosition() + vL * collisionMS);
			mBall.setPosition(mBall.getPosition() + mBall.getVelocity() * collisionMS);
			switch (collision.candidate.rhs) {
			case CandidateType::BWALL:
				mRightPaddle.setPosition({ mRightPaddle.getPosition().getX(), mLowerWall.getAABB().getMinY() - mRightPaddle.getExtent().getY() - .001f });
				vR.setY(0.f);
				break;
			case CandidateType::TWALL:
				mRightPaddle.setPosition({ mRightPaddle.getPosition().getX(), mUpperWall.getAABB().getMaxY() + mRightPaddle.getExtent().getY() + .001f });
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
	mBall.setVelocity({ -mBall.getVelocity().getX(), -mBall.getVelocity().getY() });

	Vec2f lPosition = mLeftPaddle.getPosition();
	Vec2f rPosition = mRightPaddle.getPosition();
	lPosition.setY(.5f);
	rPosition.setY(.5f);
	mLeftPaddle.setPosition(lPosition);
	mRightPaddle.setPosition(rPosition);
}
