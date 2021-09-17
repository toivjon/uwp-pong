#pragma once

#include "rectangle.h"
#include "renderable.h"
#include "text.h"

class Scene : public Renderable {
public:
	using Ptr = std::unique_ptr<Scene>;

	Scene();
	void update(std::chrono::milliseconds delta);
	void render(const Renderer::Ptr& renderer) const final;
	
	void setLeftPaddleYVelocity(float yVelocity) { 
		Vec2f velocity = mLeftPaddle.getVelocity();
		velocity.setY(yVelocity);
		mLeftPaddle.setVelocity(velocity);
	}

	void setRightPaddleYVelocity(float yVelocity) {
		Vec2f velocity = mRightPaddle.getVelocity();
		velocity.setY(yVelocity);
		mRightPaddle.setVelocity(velocity);
	}

	auto getRightPaddleVelocity() -> const Vec2f&{ return mRightPaddle.getVelocity(); }
	auto getLeftPaddleVelocity() -> const Vec2f& { return mLeftPaddle.getVelocity(); }
private:
	void resetGame();

	Rectangle	mBall;
	Rectangle	mUpperWall;
	Rectangle	mLowerWall;
	Rectangle	mLeftPaddle;
	Rectangle	mRightPaddle;
	Rectangle   mLeftGoal;
	Rectangle   mRightGoal;
	Text		mLeftScore;
	Text		mRightScore;
};