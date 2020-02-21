#include "component.h"
#include "context.h"
#include "entity.h"

using namespace pong;

Entity::Entity() : mPosition({ 0, 0 })
{
	// ...
}

Entity::~Entity()
{
	RemoveAllComponents();
}

void Entity::AddComponent(std::shared_ptr<Component> component)
{
	if (!HasComponent(component)) {
		mComponents.push_back(component);
		component->SetEntity(shared_from_this());
	}
}

void Entity::AddComponents(std::vector<std::shared_ptr<Component>> components)
{
	for (auto component : components) {
		AddComponent(component);
	}
}

void Entity::RemoveComponent(std::shared_ptr<Component> component)
{
	for (auto i = 0u; i < mComponents.size(); i++) {
		if (mComponents[i] == component) {
			mComponents.erase(mComponents.begin() + i);
			std::shared_ptr<Entity> parent = component->GetEntity().lock();
			if (parent || parent.get() == this) {
				component->SetEntity(nullptr);
			}
			break;
		}
	}
}

void Entity::RemoveComponents(std::vector<std::shared_ptr<Component>> components)
{
	for (auto component : components) {
		RemoveComponent(component);
	}
}

void Entity::RemoveAllComponents()
{
	// detach components quickly by taking full control of the list.
	std::vector<std::shared_ptr<Component>> components;
	components.swap(mComponents);
	for (auto component : components) {
		component->SetEntity(nullptr);
	}
}

bool Entity::HasComponent(std::shared_ptr<Component> component)
{
	for (auto it : mComponents) {
		if (it == component) {
			return true;
		}
	}
	return false;
}
