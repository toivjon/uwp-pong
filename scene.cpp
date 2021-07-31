#include "pch.h"
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

	mLowerWall.setBrush(brush);
	mLowerWall.setHeight(.03f);
	mLowerWall.setWidth(1.f);
	mLowerWall.setX(0.5f);
	mLowerWall.setY(.985f);

	mLeftPaddle.setBrush(brush);
	mLeftPaddle.setHeight(.15f);
	mLeftPaddle.setWidth(.025f);
	mLeftPaddle.setX(0.05f);
	mLeftPaddle.setY(0.2f);

	mRightPaddle.setBrush(brush);
	mRightPaddle.setHeight(.15f);
	mRightPaddle.setWidth(.025f);
	mRightPaddle.setX(.95f);
	mRightPaddle.setY(0.8f);
	
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
	static auto directionX = .5f;
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