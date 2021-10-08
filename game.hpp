#pragma once

#include "audio.hpp"
#include "renderer.hpp"

class Game {
public:
	using Ptr = std::unique_ptr<Game>;

	Game(Audio::Ptr& audio);
	void update(std::chrono::milliseconds delta) { state->update(*this, delta); }
	void render(const Renderer::Ptr& renderer) { state->render(*this, renderer); }
	void onKeyDown(const winrt::Windows::UI::Core::KeyEventArgs& args) { state->onKeyDown(*this, args); }
	void onKeyUp(const winrt::Windows::UI::Core::KeyEventArgs& args) { state->onKeyUp(*this, args); }
	void onReadGamepad(int player, const winrt::Windows::Gaming::Input::GamepadReading& reading) { state->onReadGamepad(*this, player, reading); }
private:
	enum class MoveDirection { NONE, UP, DOWN };

	class State {
	public:
		using Ref = std::shared_ptr<State>;
		virtual void update(Game& game, std::chrono::milliseconds delta) = 0;
		virtual void render(Game& game, const Renderer::Ptr& renderer) = 0;
		virtual void onKeyDown(Game& game, const winrt::Windows::UI::Core::KeyEventArgs& args) = 0;
		virtual void onKeyUp(Game& game, const winrt::Windows::UI::Core::KeyEventArgs& args) = 0;
		virtual void onReadGamepad(Game& game, int player, const winrt::Windows::Gaming::Input::GamepadReading& reading) = 0;
	};

	class DialogState final : public State {
	public:
		DialogState(const std::wstring& description);
		void update(Game&, std::chrono::milliseconds) override {};
		void render(Game& game, const Renderer::Ptr& renderer) override;
		void onKeyDown(Game& game, const winrt::Windows::UI::Core::KeyEventArgs& args) override;
		void onKeyUp(Game&, const winrt::Windows::UI::Core::KeyEventArgs&) override {};
		void onReadGamepad(Game& game, int player, const winrt::Windows::Gaming::Input::GamepadReading& reading) override;
		void startGame(Game& game);
	private:
		Rectangle   background;
		Rectangle   foreground;
		Text        topic;
		Text        description;
	};

	class CountdownState final : public State {
	public:
		CountdownState(Game& game);
		void update(Game& game, std::chrono::milliseconds delta) override;
		void render(Game& game, const Renderer::Ptr& renderer) override;
		void onKeyDown(Game&, const winrt::Windows::UI::Core::KeyEventArgs&) override {};
		void onKeyUp(Game&, const winrt::Windows::UI::Core::KeyEventArgs&) override {};
		void onReadGamepad(Game&, int, const winrt::Windows::Gaming::Input::GamepadReading&) override {};
	private:
		int countdown = 50;
	};

	class PlayState final : public State {
	public:
		void update(Game& game, std::chrono::milliseconds delta) override;
		void render(Game& game, const Renderer::Ptr& renderer) override;
		void onKeyDown(Game& game, const winrt::Windows::UI::Core::KeyEventArgs& args) override;
		void onKeyUp(Game& game, const winrt::Windows::UI::Core::KeyEventArgs& args) override;
		void onReadGamepad(Game& game, int player, const winrt::Windows::Gaming::Input::GamepadReading& reading) override;
		void applyMoveDirection(Rectangle& rect, MoveDirection direction);
	private:
		MoveDirection player1MoveDirection = MoveDirection::NONE;
		MoveDirection player2MoveDirection = MoveDirection::NONE;
	};

	Audio::Ptr& audio;
	State::Ref  state;

	int player1Score = 0;
	int player2Score = 0;

	struct Collision {
		ObjectID lhs = ObjectID::NONE;
		ObjectID rhs = ObjectID::NONE;
		float	 time = FLT_MAX;
	};

	auto detectCollision(float deltaMS) const->Collision;
	void detectCollision(float deltaMS, const Rectangle& r1, const Rectangle& r2, Collision& result) const;
	auto detectCollision(float deltaMS, const Rectangle& r1, const Rectangle& r2) const->Collision;

	void resolveCollision(const Collision& collision);

	Rectangle	mBall;
	Rectangle	mTopWall;
	Rectangle	mBottomWall;
	Rectangle	mLeftPaddle;
	Rectangle	mRightPaddle;
	Rectangle   mLeftGoal;
	Rectangle   mRightGoal;
	Text		mLeftScore;
	Text		mRightScore;

	bool mNewRound = false;

	Audio::Sound mBeepSound;
};