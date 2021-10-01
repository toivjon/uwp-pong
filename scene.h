#pragma once

#include "audio.hpp"
#include "renderer.h"

class Scene {
public:
	using Ptr = std::unique_ptr<Scene>;

	Scene(const Renderer::Ptr& renderer, Audio::Ptr& audio);
	void update(std::chrono::milliseconds delta);
	void render(const Renderer::Ptr& renderer) const;
	void onKeyDown(const winrt::Windows::UI::Core::KeyEventArgs& args);
	void onKeyUp(const winrt::Windows::UI::Core::KeyEventArgs& args);
	void onReadGamepad(int player, const winrt::Windows::Gaming::Input::GamepadReading& reading);

private:
	void newRound();
	void newGame();
	void clearScores();

	enum class CandidateType { LPADDLE, RPADDLE, BALL, TWALL, BWALL, LGOAL, RGOAL };

	struct Candidate {
		CandidateType lhs;
		CandidateType rhs;
	};

	struct NarrowCDResult {
		bool		hasHit;
		float		hitTime;
		Candidate	candidate;
	};
	auto broadCD(const Vec2f& pL, const Vec2f& pR, const Vec2f& pB)const->std::vector<Candidate>;

	auto narrowCD(const std::vector<Candidate>& candidates, const Vec2f& vL, const Vec2f& vR, float deltaMS)const->NarrowCDResult;

	struct GameContext {
		int Countdown = 0;
		int P1Score   = 0;
		int P2Score   = 0;
	};

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

	GameContext ctx;

	Audio::Ptr&  mAudio;
	Audio::Sound mBeepSound;
};