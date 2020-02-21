#pragma once

#include <d2d1_3.h>
#include <memory>
#include <vector>

namespace pong
{
	// forward declarations
	class Component;

	// ========================================================================

	class Entity final : public std::enable_shared_from_this<Entity>
	{
	public:
		Entity();
		Entity(Entity&) = delete;
		Entity(Entity&&) = delete;

		Entity& operator=(Entity&) = delete;
		Entity& operator=(Entity&&) = delete;

		~Entity();

		void AddComponent(std::shared_ptr<Component> component);
		void AddComponents(std::vector<std::shared_ptr<Component>> components);
		void RemoveComponent(std::shared_ptr<Component> component);
		void RemoveComponents(std::vector<std::shared_ptr<Component>> components);
		void RemoveAllComponents();
		bool HasComponent(std::shared_ptr<Component> component);
		
		void SetPosition(const D2D1_SIZE_F& position) { mPosition = position; }
		const D2D1_SIZE_F& GetPosition() const		  { return mPosition;	  }
			  D2D1_SIZE_F& GetPosition()			  { return mPosition;	  }
	private:
		std::vector<std::shared_ptr<Component>> mComponents;
		D2D1_SIZE_F mPosition;
	};

	// ========================================================================
}
