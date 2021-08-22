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
}

void Scene::update(std::chrono::milliseconds delta) {
	auto hasCollision = false;
	do {
		// TODO build AABBs with the delta and form an octree
		// TODO use octree to check if there is collision candidates (broad phase)
		// TODO if (collisionCandidates > 0)
		// TODO    perform narrow collision detection for candidates
		// TODO    if (collisions > 0)
		// TODO       resolve collision with the lowest alpha
		// TODO       apply movement to all items based on the alpha
		// TODO       delta -= alpha
	} while (hasCollision);

	// TODO How about a bit more elegant way to determine which objects to update?
	mBall.setPosition(mBall.getPosition() + mBall.getVelocity() * delta.count());
	mLeftPaddle.setPosition(mLeftPaddle.getPosition() + mLeftPaddle.getVelocity() * delta.count());
	mRightPaddle.setPosition(mRightPaddle.getPosition() + mRightPaddle.getVelocity() * delta.count());

	if (mBall.getPosition().getX() <= 0.f) {
		auto velocity = mBall.getVelocity();
		mBall.setVelocity({ -velocity.getX(), velocity.getY() });
	} else if (mBall.getPosition().getX() >= 1.f) {
		auto velocity = mBall.getVelocity();
		mBall.setVelocity({ -velocity.getX(), velocity.getY() });
	}
	if (mBall.getPosition().getY() <= 0.f) {
		auto velocity = mBall.getVelocity();
		mBall.setVelocity({ velocity.getX(), -velocity.getY() });
	} else if (mBall.getPosition().getY() >= 1.f) {
		auto velocity = mBall.getVelocity();
		mBall.setVelocity({ velocity.getX(), -velocity.getY() });
	}
	/*
	const static auto paddleVelocity = .00025f;
	static auto paddleDirectionY = -1.f;

	const auto paddleMovement = paddleDirectionY * paddleVelocity * delta.count();

	const auto paddleAABB = AABB(
		{ mRightPaddle.getPreviousPosition().getX(), mRightPaddle.getPreviousPosition().getY() },
		{ mRightPaddle.getSize().getX() / 2.f, mRightPaddle.getSize().getY() / 2.f }
	);
	const auto topWallAABB = AABB(
		{ mUpperWall.getPosition().getX(), mUpperWall.getPosition().getY() },
		{ mUpperWall.getSize().getX() / 2.f, mUpperWall.getSize().getY() / 2.f }
	);
	const auto bottomWallAABB = AABB(
		{ mLowerWall.getPosition().getX(), mLowerWall.getPosition().getY() },
		{ mLowerWall.getSize().getX() / 2.f, mLowerWall.getSize().getY() / 2.f }
	);
	auto v1 = Vec2f(0.f, paddleMovement);
	const static auto nudge = .01f;
	auto v2 = Vec2f{ 0.f, 0.f };

	auto c1 = AABB::intersect(paddleAABB, topWallAABB, v1, v2);
	if (c1.collides) {
		paddleDirectionY = 1.f;
		const auto wallMaxY = topWallAABB.getMax(1);
		const auto nudgeAmount = paddleDirectionY * nudge;
		const auto scalar = 1.f - c1.time;
		const auto leftoverMovement = -1.f * paddleMovement * scalar;
		const auto newY = wallMaxY + nudgeAmount + leftoverMovement;
		mRightPaddle.setPosition({ mRightPaddle.getPosition().getX(), newY + mRightPaddle.getSize().getY() / 2.f });
	}
	auto c2 = AABB::intersect(paddleAABB, bottomWallAABB, v1, v2);
	if (c2.collides) {
		paddleDirectionY = -1.f;
		const auto wallMinY = bottomWallAABB.getMin(1);
		const auto nudgeAmount = paddleDirectionY * nudge;
		const auto scalar = 1.f - c2.time;
		const auto leftoverMovement = -1.f * paddleMovement * scalar;
		const auto newY = wallMinY + nudgeAmount + leftoverMovement;
		mRightPaddle.setPosition({ mRightPaddle.getPosition().getX(), newY - mRightPaddle.getSize().getY() / 2.f });
	}

	// This is here just to ensure that paddle actually moves when theres no collision.
	if (!c1.collides && !c2.collides) {
		mRightPaddle.setPosition({ mRightPaddle.getPosition().getX(), mRightPaddle.getPosition().getY() + paddleMovement });
	}
	*/
	// TODO update ball
	// TODO update left paddle
	// TODO update right paddle
	// TODO check collisions?
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