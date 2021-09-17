#include "pch.h"
#include "aabb.h"
#include "scene.h"

Scene::Scene() {
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

	mLeftGoal.setSize({ 1.f, 1.f });
	mLeftGoal.setPosition({ -.5f - mBall.getSize().getX() * 2.f, .5f });

	mRightGoal.setSize({ 1.f, 1.f });
	mRightGoal.setPosition({ 1.5f + mBall.getSize().getX() * 2.f, .5f });
}

void Scene::update(std::chrono::milliseconds delta) {
	// The time (in milliseconds) we consume during the simulation step.
	auto deltaMS = static_cast<float>(delta.count());

	// Pre-build AABBs for non-moving (static) entities.
	const auto uwAABB = AABB(mUpperWall.getPosition(), mUpperWall.getSize() / 2.f);
	const auto lwAABB = AABB(mLowerWall.getPosition(), mLowerWall.getSize() / 2.f);
	const auto lgAABB = AABB(mLeftGoal.getPosition(), mLeftGoal.getSize() / 2.f);
	const auto rgAABB = AABB(mRightGoal.getPosition(), mRightGoal.getSize() / 2.f);

	// Gather velocities and prevent input to change paddle velocities on-the-fly.
	Vec2f lVelocity = mLeftPaddle.getVelocity();
	Vec2f rVelocity = mRightPaddle.getVelocity();
	Vec2f bVelocity = mBall.getVelocity();

	// Gather the current positions of each dynamic entity.
	Vec2f lPosition = mLeftPaddle.getPosition();
	Vec2f rPosition = mRightPaddle.getPosition();
	Vec2f bPosition = mBall.getPosition();

	// TODO A temporary solution which should be handled in a more elegant way.
	auto mustResetGame = false;

	// TODO We could have these precalculated in the Rectangle class?
	// Gather the extents of the dynamic entities.
	const auto lExtent = mLeftPaddle.getSize() / 2.f;
	const auto rExtent = mRightPaddle.getSize() / 2.f;
	const auto bExtent = mBall.getSize() / 2.f;

	auto hasCollision = false;
	do {
		// Build AABBs for dynamic entities based on their current positions.
		const auto lAABB = AABB(lPosition, lExtent);
		const auto rAABB = AABB(rPosition, rExtent);
		const auto bAABB = AABB(bPosition, bExtent);

		// Calculate the new ideal positions for dynamic entities.
		lPosition += lVelocity * deltaMS;
		rPosition += rVelocity * deltaMS;
		bPosition += bVelocity * deltaMS;

		// Build AABBs for dynamic entities based on their current and ideal positions.
		auto dlAABB = lAABB + AABB(lPosition, lExtent);
		auto drAABB = rAABB + AABB(rPosition, rExtent);
		auto dbAABB = bAABB + AABB(bPosition, bExtent);

		enum class CandidateType { LPADDLE, RPADDLE, BALL, TWALL, BWALL, LGOAL, RGOAL };
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
			static const Vec2f NoVelocity = { 0.f, 0.f };
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
						hit = AABB::intersect(bAABB, lAABB, bVelocity, lVelocity);
						break;
					case CandidateType::RPADDLE:
						hit = AABB::intersect(bAABB, rAABB, bVelocity, rVelocity);
						break;
					case CandidateType::TWALL:
						hit = AABB::intersect(bAABB, uwAABB, bVelocity, NoVelocity);
						break;
					case CandidateType::BWALL:
						hit = AABB::intersect(bAABB, lwAABB, bVelocity, NoVelocity);
						break;
					case CandidateType::LGOAL:
						hit = AABB::intersect(bAABB, lgAABB, bVelocity, NoVelocity);
						break;
					case CandidateType::RGOAL:
						hit = AABB::intersect(bAABB, rgAABB, bVelocity, NoVelocity);
						break;
					}
					break;
				case CandidateType::LPADDLE:
					switch (candidate.rhs) {
					case CandidateType::TWALL:
						hit = AABB::intersect(lAABB, uwAABB, lVelocity, NoVelocity);
						break;
					case CandidateType::BWALL:
						hit = AABB::intersect(lAABB, lwAABB, lVelocity, NoVelocity);
						break;
					}
					break;
				case CandidateType::RPADDLE:
					switch (candidate.rhs) {
					case CandidateType::TWALL:
						hit = AABB::intersect(rAABB, uwAABB, rVelocity, NoVelocity);
						break;
					case CandidateType::BWALL:
						hit = AABB::intersect(rAABB, lwAABB, rVelocity, NoVelocity);
						break;
					}
					break;
				}
				if (hit.collides && hit.time < minTime) {
					minTime = std::max(hit.time, 0.f);
					pair = candidate;
					hasHit = true;
				}
			}

			// React to collision that was detected as the soonest collision.
			if (hasHit) {
				// TODO       apply movement to all items based on the alpha
				minTime -= 0.0001f;
				lPosition = mLeftPaddle.getPosition();
				rPosition = mRightPaddle.getPosition();
				bPosition = mBall.getPosition();
				lPosition += lVelocity * minTime;
				rPosition += rVelocity * minTime;
				bPosition += bVelocity * minTime;

				deltaMS -= minTime;

				if (pair.lhs == CandidateType::BALL) {
					switch (pair.rhs) {
					case CandidateType::TWALL:
						bVelocity.setY(-bVelocity.getY());
						break;
					case CandidateType::BWALL:
						bVelocity.setY(-bVelocity.getY());
						break;
					case CandidateType::LPADDLE:
						bVelocity.setX(-bVelocity.getX());
						break;
					case CandidateType::RPADDLE:
						bVelocity.setX(-bVelocity.getX());
						break;
					case CandidateType::LGOAL:
						// TODO Implement scoring logic.
						mustResetGame = true;
						break;
					case CandidateType::RGOAL:
						// TODO Implement scoring logic.
						mustResetGame = true;
						break;
					}
				} else if (pair.lhs == CandidateType::LPADDLE) {
					lPosition.setY(lPosition.getY() + .0001f * -lVelocity.getY());
					lVelocity.setY(0.f);
				} else if (pair.lhs == CandidateType::RPADDLE) {
					rPosition.setY(rPosition.getY() + .0001f * -rVelocity.getY());
					rVelocity.setY(0.f);
				}
			}
		}

		// Apply new velocity directions to entities.
		// mLeftPaddle.setVelocity(lVelocity);
		// mRightPaddle.setVelocity(rVelocity);
		mBall.setVelocity(bVelocity);

		// Apply new positions to dynamic entities.
		mLeftPaddle.setPosition(lPosition);
		mRightPaddle.setPosition(rPosition);
		mBall.setPosition(bPosition);
	} while (hasCollision);

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
	mBall.setPosition({ .5f, .5f });
	mBall.setVelocity({ -mBall.getVelocity().getX(), -mBall.getVelocity().getY() });

	Vec2f lPosition = mLeftPaddle.getPosition();
	Vec2f rPosition = mRightPaddle.getPosition();
	lPosition.setY(.5f);
	rPosition.setY(.5f);
	mLeftPaddle.setPosition(lPosition);
	mRightPaddle.setPosition(rPosition);
}
