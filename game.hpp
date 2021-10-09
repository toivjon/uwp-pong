#pragma once

#include "audio.hpp"
#include "renderer.hpp"

class Game {
public:
	Game(const Audio& audio);
	void update(std::chrono::milliseconds delta) { state->update(delta); }
	void render(const Renderer& renderer) { state->render(renderer); }
	void onKeyDown(const winrt::Windows::UI::Core::KeyEventArgs& args) { state->onKeyDown(args); }
	void onKeyUp(const winrt::Windows::UI::Core::KeyEventArgs& args) { state->onKeyUp(args); }
	void onReadGamepad(int player, const winrt::Windows::Gaming::Input::GamepadReading& reading) { state->onReadGamepad(player, reading); }
private:
	class State {
	public:
		State(Game& gameRef) : game(gameRef) {}
		virtual void update(std::chrono::milliseconds delta) = 0;
		virtual void render(const Renderer& renderer) = 0;
		virtual void onKeyDown(const winrt::Windows::UI::Core::KeyEventArgs& args) = 0;
		virtual void onKeyUp(const winrt::Windows::UI::Core::KeyEventArgs& args) = 0;
		virtual void onReadGamepad(int player, const winrt::Windows::Gaming::Input::GamepadReading& reading) = 0;
	protected:
		Game& game;
	};

	class DialogState final : public State {
	public:
		DialogState(Game& game, const std::wstring& description);
		void update(std::chrono::milliseconds) override {};
		void render(const Renderer& renderer) override;
		void onKeyDown(const winrt::Windows::UI::Core::KeyEventArgs& args) override;
		void onKeyUp(const winrt::Windows::UI::Core::KeyEventArgs&) override {};
		void onReadGamepad(int player, const winrt::Windows::Gaming::Input::GamepadReading& reading) override;
		void startGame();
	private:
		Rectangle::Ref background;
		Rectangle::Ref foreground;
		Text::Ref      topic;
		Text::Ref      description;
	};

	class CountdownState final : public State {
	public:
		CountdownState(Game& game);
		void update(std::chrono::milliseconds delta) override;
		void render(const Renderer& renderer) override;
		void onKeyDown(const winrt::Windows::UI::Core::KeyEventArgs&) override {};
		void onKeyUp(const winrt::Windows::UI::Core::KeyEventArgs&) override {};
		void onReadGamepad(int, const winrt::Windows::Gaming::Input::GamepadReading&) override {};
	private:
		auto newRandomDirection()->Vec2f;
		int countdown = 50;
	};

	class PlayState final : public State {
	public:
		enum class MoveDirection { UP = -1, NONE = 0, DOWN = 1};
		PlayState(Game& game) : State(game) {}
		void update(std::chrono::milliseconds delta) override;
		void render(const Renderer& renderer) override;
		void onKeyDown(const winrt::Windows::UI::Core::KeyEventArgs& args) override;
		void onKeyUp(const winrt::Windows::UI::Core::KeyEventArgs& args) override;
		void onReadGamepad(int player, const winrt::Windows::Gaming::Input::GamepadReading& reading) override;
	private:
		MoveDirection player1Movement = MoveDirection::NONE;
		MoveDirection player2Movement = MoveDirection::NONE;
	};

	std::shared_ptr<State> state;

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