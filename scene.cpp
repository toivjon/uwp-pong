#include "pch.h"
#include "scene.h"

Scene::Scene(const Renderer::Ptr& renderer) {
	winrt::com_ptr<ID2D1SolidColorBrush> brush;
	renderer->getD2DContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), brush.put());

	mSphere.setBrush(brush);
	mSphere.setRadius(.01f);
	mSphere.setX(.5f);
	mSphere.setY(.5f);

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

void Scene::update(float /*dt*/) {
	// TODO update ball
	// TODO update left paddle
	// TODO update right paddle
	// TODO check collisions?
}

void Scene::render(const Renderer::Ptr& renderer) const {
	mLeftScore.render(renderer);
	mRightScore.render(renderer);
	mSphere.render(renderer);
	mUpperWall.render(renderer);
	mLowerWall.render(renderer);
	mLeftPaddle.render(renderer);
	mRightPaddle.render(renderer);
}