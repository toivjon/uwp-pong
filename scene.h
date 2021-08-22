#pragma once

#include "rectangle.h"
#include "renderable.h"
#include "text.h"

class Scene : public Renderable {
public:
	using Ptr = std::unique_ptr<Scene>;

	Scene(const Renderer::Ptr& renderer);
	void update(std::chrono::milliseconds delta);
	void render(const Renderer::Ptr& renderer) const final;
private:
	Rectangle	mBall;
	Rectangle	mUpperWall;
	Rectangle	mLowerWall;
	Rectangle	mLeftPaddle;
	Rectangle	mRightPaddle;
	Text		mLeftScore;
	Text		mRightScore;
};