#pragma once

#include <memory>

namespace pong
{
	class Entity
	{
	public:
		// An enumeration of all available entities.
		enum class Type {
			Paddle,
			Text,
			Wall,
			Goal,
			Ball,
			CenterLine
		};

		// A factory function to build new entities.
		static std::shared_ptr<Entity> Create(Entity::Type);
	};
}
