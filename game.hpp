#pragma once

#include "audio.hpp"
#include "renderer.hpp"

class Game {
public:
	Game(Audio::Ptr& audio);
	void update(std::chrono::milliseconds delta) { state->update(*this, delta); }
	void render(const Renderer& renderer) { state->render(*this, renderer); }
	void onKeyDown(const winrt::Windows::UI::Core::KeyEventArgs& args) { state->onKeyDown(*this, args); }
	void onKeyUp(const winrt::Windows::UI::Core::KeyEventArgs& args) { state->onKeyUp(*this, args); }
	void onReadGamepad(int player, const winrt::Windows::Gaming::Input::GamepadReading& reading) { state->onReadGamepad(*this, player, reading); }
private:
	class State {
	public:
		using Ref = std::shared_ptr<State>;
		virtual void update(Game& game, std::chrono::milliseconds delta) = 0;
		virtual void render(Game& game, const Renderer& renderer) = 0;
		virtual void onKeyDown(Game& game, const winrt::Windows::UI::Core::KeyEventArgs& args) = 0;
		virtual void onKeyUp(Game& game, const winrt::Windows::UI::Core::KeyEventArgs& args) = 0;
		virtual void onReadGamepad(Game& game, int player, const winrt::Windows::Gaming::Input::GamepadReading& reading) = 0;
	};

	class DialogState final : public State {
	public:
		DialogState(const std::wstring& description);
		void update(Game&, std::chrono::milliseconds) override {};
		void render(Game& game, const Renderer& renderer) override;
		void onKeyDown(Game& game, const winrt::Windows::UI::Core::KeyEventArgs& args) override;
		void onKeyUp(Game&, const winrt::Windows::UI::Core::KeyEventArgs&) override {};
		void onReadGamepad(Game& game, int player, const winrt::Windows::Gaming::Input::GamepadReading& reading) override;
		void startGame(Game& game);
	private:
		Rectangle::Ref background;
		Rectangle::Ref foreground;
		Text::Ref      topic;
		Text::Ref      description;
	};

	class CountdownState final : public State {
	public:
		CountdownState(Game& game);
		void update(Game& game, std::chrono::milliseconds delta) override;
		void render(Game& game, const Renderer& renderer) override;
		void onKeyDown(Game&, const winrt::Windows::UI::Core::KeyEventArgs&) override {};
		void onKeyUp(Game&, const winrt::Windows::UI::Core::KeyEventArgs&) override {};
		void onReadGamepad(Game&, int, const winrt::Windows::Gaming::Input::GamepadReading&) override {};
	private:
		auto newRandomDirection()->Vec2f;
		int countdown = 50;
	};

	class PlayState final : public State {
	public:
		enum class MoveDirection { UP = -1, NONE = 0, DOWN = 1};
		void update(Game& game, std::chrono::milliseconds delta) override;
		void render(Game& game, const Renderer& renderer) override;
		void onKeyDown(Game& game, const winrt::Windows::UI::Core::KeyEventArgs& args) override;
		void onKeyUp(Game& game, const winrt::Windows::UI::Core::KeyEventArgs& args) override;
		void onReadGamepad(Game& game, int player, const winrt::Windows::Gaming::Input::GamepadReading& reading) override;
	private:
		MoveDirection player1Movement = MoveDirection::NONE;
		MoveDirection player2Movement = MoveDirection::NONE;
	};

	State::Ref state;

	int player1Score = 0;
	int player2Score = 0;

	struct Collision {
		Rectangle::Ref lhs;
		Rectangle::Ref rhs;
		float          time = FLT_MAX;
	};

	auto detectCollision(float deltaMS) const->Collision;
	void detectCollision(float deltaMS, Rectangle::Ref r1, Rectangle::Ref r2, Collision& result) const;
	auto detectCollision(float deltaMS, Rectangle::Ref r1, Rectangle::Ref r2) const->Collision;

	void resolveCollision(const Collision& collision);

	Rectangle::Ref ball;
	Rectangle::Ref topWall;
	Rectangle::Ref bottomWall;
	Rectangle::Ref leftPaddle;
	Rectangle::Ref rightPaddle;
	Rectangle::Ref leftGoal;
	Rectangle::Ref rightGoal;
	Text::Ref      leftScore;
	Text::Ref      rightScore;

	bool newRound = false;

	Audio::Sound beepSound;
};