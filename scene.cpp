#include "scene.h"
#include "component.h"
#include "context.h"
#include "entity.h"

#include <unordered_map>

using namespace pong;

void Scene::OnEnter(Context& ctx)
{
	// ...
}

void Scene::OnExit(Context& ctx)
{
	// ...
}

void Scene::Render(RenderingContext& ctx)
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
	void OnEnter(Context& ctx)
	{
		mTitle = std::make_shared<Entity>();
		mTitle->SetPosition(D2D1_SIZE_F{ 100, 100 }); // TODO relative to window size
		mTitle->AddComponent(std::make_shared<TextComponent>(L"PONG"));

		mLeftPlayerText = std::make_shared<Entity>();
		mLeftPlayerText->SetPosition(D2D1_SIZE_F{ 50, 200 }); // TODO relative to window size
		mLeftPlayerText->AddComponent(std::make_shared<TextComponent>(L"Left Player"));

		mRightPlayerText = std::make_shared<Entity>();
		mRightPlayerText->SetPosition(D2D_SIZE_F{ 400, 200 }); // TODO relative to window size
		mRightPlayerText->AddComponent(std::make_shared<TextComponent>(L"Right Player"));

		mTopWall = std::make_shared<Entity>();
		mTopWall->SetPosition(D2D_SIZE_F{ 0, 880 }); // TODO relative to window size
		mTopWall->AddComponent(std::make_shared<RectangleComponent>(D2D1_RECT_F{ 0, 0, 1280, 20 }));

		mBottomWall = std::make_shared<Entity>();
		mBottomWall->SetPosition(D2D_SIZE_F{ 0, 0 }); // TODO relative to window size
		mBottomWall->AddComponent(std::make_shared<RectangleComponent>(D2D1_RECT_F{ 0, 0, 1280, 20 }));

		mRightCenterLine = std::make_shared<Entity>();
		mRightCenterLine->SetPosition(D2D_SIZE_F{}); // TODO relative to window size
		mRightCenterLine->AddComponent(std::make_shared<RectangleComponent>(D2D1_RECT_F{ 0, 0, 100, 100 }));

		mLeftCenterLine = std::make_shared<Entity>();
		mLeftCenterLine->SetPosition(D2D_SIZE_F{}); // TODO relative to window size
		mLeftCenterLine->AddComponent(std::make_shared<RectangleComponent>(D2D1_RECT_F{ 0, 0, 100, 100 }));
	}

	void Render(RenderingContext& ctx)
	{
		mTitle->Render(ctx);
		mLeftPlayerText->Render(ctx);
		mRightPlayerText->Render(ctx);
		mTopWall->Render(ctx);
		mBottomWall->Render(ctx);
		mRightCenterLine->Render(ctx);
		mLeftCenterLine->Render(ctx);
	}
private:
	std::shared_ptr<Entity> mTitle;
	std::shared_ptr<Entity> mLeftPlayerText;
	std::shared_ptr<Entity> mRightPlayerText;
	std::shared_ptr<Entity> mTopWall;
	std::shared_ptr<Entity> mBottomWall;
	std::shared_ptr<Entity> mRightCenterLine;
	std::shared_ptr<Entity> mLeftCenterLine;
};

class Court : public Scene
{
public:

};

class GameOver : public Scene
{
public:

};

std::shared_ptr<Scene> Scene::Get(Scene::ID id)
{
	static std::unordered_map<Scene::ID, std::shared_ptr<Scene>> mScenes;
	if (mScenes.find(id) == mScenes.end()) {
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