#include "scene.h"
#include "context.h"

using namespace pong;

void Scene::OnEnter(Context& ctx)
{
	// ...
}

void Scene::OnExit(Context& ctx)
{
	// ...
}

void Scene::OnRender(Context& ctx)
{
	// ...
}

void Scene::OnUpdate(Context& ctx)
{
	// ...
}

std::shared_ptr<Scene> Scene::Create(Scene::ID id)
{
	static std::unordered_map<Scene::ID, std::shared_ptr<Scene>> mScenes;
	auto it = mScenes.find(id);
	if (it == mScenes.end()) {
		mScenes[id] = std::make_shared<Scene>();
	}
	return mScenes[id];
}