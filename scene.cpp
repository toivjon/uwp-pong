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

class MainMenu : public Scene
{
public:
	
};

class Court : public Scene
{
public:

};

class GameOver : public Scene
{
public:

};

std::shared_ptr<Scene> Scene::Create(Scene::ID id)
{
	static std::unordered_map<Scene::ID, std::shared_ptr<Scene>> mScenes;
	auto it = mScenes.find(id);
	if (it == mScenes.end()) {
		switch (id) {
		case ID::MainMenu:
			mScenes[id] = std::make_shared<MainMenu>();
			break;
		case ID::Court:
			mScenes[id] = std::make_shared<Court>();
			break;
		case ID::GameOver:
			mScenes[id] = std::make_shared<GameOver>();
			break;
		default:
			break;
		}
	}
	return mScenes[id];
}