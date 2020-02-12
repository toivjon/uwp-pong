#pragma once

#include <string>
#include <variant>

namespace pong
{
	enum class EventType {
		Unknown,
		ChangeScene,
		GamepadAdded,
		GamepadRemoved
	};

	struct ChangeSceneArgs {
		// The name of the scene to be moved into.
		std::string NextSceneName;
	};

	struct GamepadEvent {
		// A reference to target gamepad item.
		Windows::Gaming::Input::Gamepad^ Gamepad;
	};

	struct Event {
		// The priority of the event marking the order within a priority queue.
		int Priority = 0;

		// The type which indicates the purpose of the event.
		EventType Type = EventType::Unknown;

		// The type specific variadic arguments for the event.
		std::variant<ChangeSceneArgs, GamepadEvent> Args;

		// A comparator for the ordering in the priority queue.
		friend bool operator < (const Event& lhs, const Event& rhs) {
			return lhs.Priority < rhs.Priority;
		}
	};

}
