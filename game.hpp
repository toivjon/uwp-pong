#pragma once

#include "audio.hpp"
#include "renderer.hpp"

class Game {
public:
	using Ptr = std::unique_ptr<Game>;

	Game(const Renderer::Ptr& renderer, Audio::Ptr& audio);
	void update(std::chrono::milliseconds delta);
	void render(const Renderer::Ptr& renderer) const;
	void onKeyDown(const winrt::Windows::UI::Core::KeyEventArgs& args);
	void onKeyUp(const winrt::Windows::UI::Core::KeyEventArgs& args);
	void onReadGamepad(int player, const winrt::Windows::Gaming::Input::GamepadReading& reading);

private:
	void newRound();
	void newGame();

	enum class CandidateType { LPADDLE, RPADDLE, BALL, TWALL, BWALL, LGOAL, RGOAL };

	struct Candidate {
		CandidateType lhs;
		CandidateType rhs;
	};

	struct CollisionResult {
		bool		hasHit;
		float		hitTime;
		Candidate	candidate;
	};

	auto detectCollision(float deltaMS)const->CollisionResult;

	Rectangle   mDialogBackground;
	Rectangle   mDialogForeground;
	Text        mDialogTopic;
	Text        mDialogDescription;
	bool        mDialogVisible;

	Rectangle	mBall;
	Rectangle	mUpperWall;
	Rectangle	mLowerWall;
	Rectangle	mLeftPaddle;
	Rectangle	mRightPaddle;
	Rectangle   mLeftGoal;
	Rectangle   mRightGoal;
	Text		mLeftScore;
	Text		mRightScore;

	int mCountdown = 0;
	int mP1Score = 0;
	int mP2Score = 0;

	enum class MoveDirection { NONE, UP, DOWN };

	MoveDirection mP1MoveDirection = MoveDirection::NONE;
	MoveDirection mP2MoveDirection = MoveDirection::NONE;

	Audio::Ptr&  mAudio;
	Audio::Sound mBeepSound;
};