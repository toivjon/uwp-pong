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

	auto getRightPaddleVelocity() -> const Vec2f& { return mRightPaddle.getVelocity(); }
	auto getLeftPaddleVelocity() -> const Vec2f& { return mLeftPaddle.getVelocity(); }
private:
	void resetGame();

	enum class CandidateType { LPADDLE, RPADDLE, BALL, TWALL, BWALL, LGOAL, RGOAL };

	struct Candidate {
		CandidateType lhs;
		CandidateType rhs;
	};

	struct NarrowCDResult {
		bool		hasHit;
		float		hitTime;
		Candidate	candidate;
		Vec2f		normal;
	};
	auto broadCD(const Vec2f& pL, const Vec2f& pR, const Vec2f& pB)const->std::vector<Candidate>;

	auto narrowCD(const std::vector<Candidate>& candidates, const Vec2f& vL, const Vec2f& vR, float deltaMS)const->NarrowCDResult;

	struct GameContext {
		int Countdown = 0;
		int P1Score   = 0;
		int P2Score   = 0;
	};

	Rectangle	mBall;
	Rectangle	mUpperWall;
	Rectangle	mLowerWall;
	Rectangle	mLeftPaddle;
	Rectangle	mRightPaddle;
	Rectangle   mLeftGoal;
	Rectangle   mRightGoal;
	Text		mLeftScore;
	Text		mRightScore;

	GameContext ctx;
};