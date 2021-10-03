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

	struct Candidate {
		CandidateType lhs;
		CandidateType rhs;
	};

	struct CollisionResult {
		bool		hasHit;
		float		hitTime;
		Candidate	candidate;
	};
private:
	enum class MoveDirection { NONE, UP, DOWN };

	void newRound();
	void newGame();
	void applyMoveDirection(Rectangle& rect, MoveDirection direction);

	auto detectCollision(float deltaMS) const->CollisionResult;
	void detectCollision(float deltaMS, const Rectangle& r1, const Rectangle& r2, CollisionResult& result) const;

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


	MoveDirection mP1MoveDirection = MoveDirection::NONE;
	MoveDirection mP2MoveDirection = MoveDirection::NONE;

	Audio::Ptr& mAudio;
	Audio::Sound mBeepSound;
};