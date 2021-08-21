#include "pch.h"
#include "aabb.h"
#include "scene.h"

Scene::Scene(const Renderer::Ptr& renderer) {
	winrt::com_ptr<ID2D1SolidColorBrush> brush;
	renderer->getD2DContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), brush.put());

	mBall.setBrush(brush);
	mBall.setRadius(.01f);
	mBall.setX(.5f);
	mBall.setY(.5f);

	mUpperWall.setBrush(brush);
	mUpperWall.setHeight(.03f);
	mUpperWall.setWidth(1.f);
	mUpperWall.setX(0.5f);
	mUpperWall.setY(0.015f);
	mUpperWall.setStatic(true);

	mLowerWall.setBrush(brush);
	mLowerWall.setHeight(.03f);
	mLowerWall.setWidth(1.f);
	mLowerWall.setX(0.5f);
	mLowerWall.setY(.985f);
	mLowerWall.setStatic(true);

	mLeftPaddle.setBrush(brush);
	mLeftPaddle.setHeight(.15f);
	mLeftPaddle.setWidth(.025f);
	mLeftPaddle.setX(0.05f);
	mLeftPaddle.setY(0.2f);
	mLeftPaddle.setStatic(false);

	mRightPaddle.setBrush(brush);
	mRightPaddle.setHeight(.15f);
	mRightPaddle.setWidth(.025f);
	mRightPaddle.setX(.95f);
	mRightPaddle.setY(0.8f);
	mRightPaddle.setStatic(false);

	mLeftScore.setText(L"0");
	mLeftScore.setBrush(brush);
	mLeftScore.setX(.35f);
	mLeftScore.setY(.025f);

	mRightScore.setText(L"0");
	mRightScore.setBrush(brush);
	mRightScore.setX(.65f);
	mRightScore.setY(.025f);
}

void Scene::update(std::chrono::milliseconds delta) {
	// TODO a temporary helper just to keep ball moving.
	const static auto velocity = .00025f;
	static auto directionX = 1.f;
	static auto directionY = 1.f;
	mBall.setX(mBall.getX() + directionX * velocity * delta.count());
	mBall.setY(mBall.getY() + directionY * velocity * delta.count());
	if (mBall.getX() <= 0.f) {
		directionX = 1.f;
	} else if (mBall.getX() >= 1.f) {
		directionX = -1.f;
	}
	if (mBall.getY() <= 0.f) {
		directionY = 1.f;
	} else if (mBall.getY() >= 1.f) {
		directionY = -1.f;
	}

	// TODO a temporary helper just to keep paddle visible.
	mLeftPaddle.setX(mLeftPaddle.getX());
	mLeftPaddle.setY(mLeftPaddle.getY());

	// TODO a temporary helper just to keep paddle visible.
	// mRightPaddle.setX(mRightPaddle.getX());
	// mRightPaddle.setY(mRightPaddle.getY());

	const static auto paddleVelocity = .00025f;
	static auto paddleDirectionY = -1.f;

	const auto paddleMovement = paddleDirectionY * paddleVelocity * delta.count();

	mRightPaddle.setX(mRightPaddle.getX());
	// mRightPaddle.setY(mRightPaddle.getY() + paddleDirectionY * paddleMovement);

	const auto paddleAABB = AABB(
		{ mRightPaddle.getPreviousX(), mRightPaddle.getPreviousY() },
		{ mRightPaddle.getHalfWidth(), mRightPaddle.getHalfHeight() }
	);
	const auto topWallAABB = AABB(
		{ mUpperWall.getX(), mUpperWall.getY() },
		{ mUpperWall.getHalfWidth(), mUpperWall.getHalfHeight() }
	);
	const auto bottomWallAABB = AABB(
		{ mLowerWall.getX(), mLowerWall.getY() },
		{ mLowerWall.getHalfWidth(), mLowerWall.getHalfHeight() }
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
		mRightPaddle.setY(newY + mRightPaddle.getHalfHeight());
	}
	auto c2 = AABB::intersect(paddleAABB, bottomWallAABB, v1, v2);
	if (c2.collides) {
		paddleDirectionY = -1.f;
		const auto wallMinY = bottomWallAABB.getMin(1);
		const auto nudgeAmount = paddleDirectionY * nudge;
		const auto scalar = 1.f - c2.time;
		const auto leftoverMovement = -1.f * paddleMovement * scalar;
		const auto newY = wallMinY + nudgeAmount + leftoverMovement;
		mRightPaddle.setY(newY - mRightPaddle.getHalfHeight());
	}

	// This is here just to ensure that paddle actually moves when theres no collision.
	if (!c1.collides && !c2.collides) {
		mRightPaddle.setY(mRightPaddle.getY() + paddleMovement);
	}

	// TODO update ball
	// TODO update left paddle
	// TODO update right paddle
	// TODO check collisions?
}

void Scene::render(float alpha, const Renderer::Ptr& renderer) const {
	mLeftScore.render(alpha, renderer);
	mRightScore.render(alpha, renderer);
	mBall.render(alpha, renderer);
	mUpperWall.render(alpha, renderer);
	mLowerWall.render(alpha, renderer);
	mLeftPaddle.render(alpha, renderer);
	mRightPaddle.render(alpha, renderer);
}