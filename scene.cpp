#include "pch.h"
#include "aabb.h"
#include "scene.h"

Scene::Scene(const Renderer::Ptr& renderer) {
	mBall.setSize({ .023f, .03f });
	mBall.setPosition({ .5f, .5f });
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

	mLeftGoal.setSize({1.f, 1.f});
	mLeftGoal.setPosition({-.5f - mBall.getSize().getX()*2.f, .5f});

	mRightGoal.setSize({1.f, 1.f});
	mRightGoal.setPosition({1.5f + mBall.getSize().getX()*2.f, .5f});
}

void Scene::update(std::chrono::milliseconds delta) {
	// The time (in milliseconds) we consume during the simulation step.
	auto deltaMS = delta.count();

	// TODO We could have these precalculated somewhere?
	// Pre-build AABBs for non-moving (static) entities.
	const auto uwAABB = AABB(mUpperWall.getPosition(), mUpperWall.getSize() / 2.f);
	const auto lwAABB = AABB(mLowerWall.getPosition(), mLowerWall.getSize() / 2.f);
	const auto lgAABB = AABB(mLeftGoal.getPosition(), mLeftGoal.getSize() / 2.f);
	const auto rgAABB = AABB(mRightGoal.getPosition(), mRightGoal.getSize() / 2.f);

	// TODO A temporary solution which should be handled in a more elegant way.
	auto resetGame = false;

	auto hasCollision = false;
	do {
		// Gather the current positions of each dynamic entity.
		auto lPosition = mLeftPaddle.getPosition();
		auto rPosition = mRightPaddle.getPosition();
		auto bPosition = mBall.getPosition();

		// TODO We could have these precalculated in the Rectangle class?
		// Gather the extents of the dynamic entities.s
		const auto lExtent = mLeftPaddle.getSize() / 2.f;
		const auto rExtent = mRightPaddle.getSize() / 2.f;
		const auto bExtent = mBall.getSize() / 2.f;

		// Build AABBs for dynamic entities based on their current positions.
		const auto lAABB = AABB(lPosition, lExtent);
		const auto rAABB = AABB(rPosition, rExtent);
		const auto bAABB = AABB(bPosition, bExtent);

		// Calculate the new ideal positions for dynamic entities.
		lPosition += mLeftPaddle.getVelocity() * deltaMS;
		rPosition += mRightPaddle.getVelocity() * deltaMS;
		bPosition += mBall.getVelocity() * deltaMS;

		// Build AABBs for dynamic entities based on their current and ideal positions.
		auto dlAABB = lAABB + AABB(lPosition, lExtent);
		auto drAABB = rAABB + AABB(rPosition, rExtent);
		auto dbAABB = bAABB + AABB(bPosition, bExtent);

		enum class CandidateType {LPADDLE, RPADDLE, BALL, TWALL, BWALL, LGOAL, RGOAL};
		struct Candidate {
			CandidateType lhs;
			CandidateType rhs;
		};

		// Use broad phase AABBs to collect simulation collision candidates.
		std::vector<Candidate> candidates;
		if (dbAABB.collides(dlAABB)) {
			candidates.push_back({ CandidateType::BALL, CandidateType::LPADDLE });
		} else if (dbAABB.collides(drAABB)) {
			candidates.push_back({ CandidateType::BALL, CandidateType::RPADDLE });
		}
		if (dbAABB.collides(uwAABB)) {
			candidates.push_back({ CandidateType::BALL, CandidateType::TWALL });
		} else if (dbAABB.collides(lwAABB)) {
			candidates.push_back({ CandidateType::BALL, CandidateType::BWALL });
		}
		if (dbAABB.collides(lgAABB)) {
			candidates.push_back({ CandidateType::BALL, CandidateType::LGOAL });
		} else if (dbAABB.collides(rgAABB)) {
			candidates.push_back({ CandidateType::BALL, CandidateType::RGOAL });
		}
		if (drAABB.collides(uwAABB)) {
			candidates.push_back({ CandidateType::RPADDLE, CandidateType::TWALL });
		} else if (drAABB.collides(lwAABB)) {
			candidates.push_back({ CandidateType::RPADDLE, CandidateType::BWALL });
		}
		if (dlAABB.collides(uwAABB)) {
			candidates.push_back({ CandidateType::LPADDLE, CandidateType::TWALL });
		} else if (dlAABB.collides(lwAABB)) {
			candidates.push_back({ CandidateType::LPADDLE, CandidateType::BWALL });
		}

		// User narrow phase AABB sweeping to check real collisions and to detect soonest collision.
		if (!candidates.empty()) {
			auto hasHit = false;
			auto minTime = FLT_MAX;
			auto pair = Candidate{};
			for (auto& candidate : candidates) {
				AABB::Intersection hit = {};
				Rectangle lhs;
				switch (candidate.lhs) {
				case CandidateType::BALL:
					switch (candidate.rhs) {
					case CandidateType::LPADDLE:
						hit = AABB::intersect(bAABB, lAABB, mBall.getVelocity(), mLeftPaddle.getVelocity());
						break;
					case CandidateType::RPADDLE:
						hit = AABB::intersect(bAABB, rAABB, mBall.getVelocity(), mRightPaddle.getVelocity());
						break;
					case CandidateType::TWALL:
						hit = AABB::intersect(bAABB, uwAABB, mBall.getVelocity(), { 0.f,0.f });
						break;
					case CandidateType::BWALL:
						hit = AABB::intersect(bAABB, lwAABB, mBall.getVelocity(), { 0.f,0.f });
						break;
					case CandidateType::LGOAL:
						hit = AABB::intersect(bAABB, lgAABB, mBall.getVelocity(), { 0.f, 0.f });
						break;
					case CandidateType::RGOAL:
						hit = AABB::intersect(bAABB, rgAABB, mBall.getVelocity(), { 0.f, 0.f });
						break;
					}
					break;
				case CandidateType::LPADDLE:
					switch (candidate.rhs) {
					case CandidateType::TWALL:
						hit = AABB::intersect(lAABB, uwAABB, mLeftPaddle.getVelocity(), { 0.f, 0.f });
						break;
					case CandidateType::BWALL:
						hit = AABB::intersect(lAABB, lwAABB, mLeftPaddle.getVelocity(), { 0.f, 0.f });
						break;
					}
					break;
				case CandidateType::RPADDLE:
					switch (candidate.rhs) {
					case CandidateType::TWALL:
						hit = AABB::intersect(rAABB, uwAABB, mRightPaddle.getVelocity(), { 0.f, 0.f });
						break;
					case CandidateType::BWALL:
						hit = AABB::intersect(rAABB, lwAABB, mRightPaddle.getVelocity(), { 0.f, 0.f });
						break;
					}
					break;
				}
				if (hit.collides && hit.time < minTime) {
					minTime = hit.time;
					pair = candidate;
					hasHit = true;
				}
			}

			// React to collision that was detected as the soonest collision.
			if (hasHit) {
				const auto lVelocity = mLeftPaddle.getVelocity();
				const auto rVelocity = mRightPaddle.getVelocity();
				const auto bVelocity = mBall.getVelocity();

				if (pair.lhs == CandidateType::BALL) {
					const auto oldPosition = mBall.getPosition();
					auto velocity = mBall.getVelocity();
					switch (pair.rhs) {
					case CandidateType::TWALL:
						velocity.setY(-velocity.getY());
						mBall.setVelocity(velocity);
						break;
					case CandidateType::BWALL:
						velocity.setY(-velocity.getY());
						mBall.setVelocity(velocity);
						break;
					case CandidateType::LPADDLE:
						velocity.setX(-velocity.getX());
						mBall.setVelocity(velocity);
						break;
					case CandidateType::RPADDLE:
						velocity.setX(-velocity.getX());
						mBall.setVelocity(velocity);
						break;
					case CandidateType::LGOAL:
						// TODO Implement scoring logic.
						resetGame = true;
						break;
					case CandidateType::RGOAL:
						// TODO Implement scoring logic.
						resetGame = true;
						break;
					}
				} else if (pair.rhs == CandidateType::LPADDLE) {
					mLeftPaddle.setVelocity({ 0.f, 0.f });
				} else if (pair.rhs == CandidateType::RPADDLE) {
					mRightPaddle.setVelocity({ 0.f, 0.f });
				}

				// TODO       apply movement to all items based on the alpha
				minTime -= 0.0001f;
				lPosition = mLeftPaddle.getPosition();
				rPosition = mRightPaddle.getPosition();
				bPosition = mBall.getPosition();
				lPosition += lVelocity * minTime;
				rPosition += rVelocity * minTime;
				bPosition += bVelocity * minTime;

				deltaMS -= minTime;
			}
		}

		// Apply new positions to dynamic entities.
		mLeftPaddle.setPosition(lPosition);
		mRightPaddle.setPosition(rPosition);
		mBall.setPosition(bPosition);
	} while (hasCollision);

	if (resetGame) {
		mBall.setPosition({ .5f, .5f });
		mBall.setVelocity({ -mBall.getVelocity().getX(), -mBall.getVelocity().getY() });
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