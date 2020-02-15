#include "context.h"

using namespace pong;

Context::Context()
{
	mAudio = std::make_unique<Audio>();
	mGraphics = std::make_unique<Graphics>();
}

void Context::Update(unsigned long dt)
{
	// get events from the current queue.
	std::priority_queue<Event> events;
	events.swap(mEvents);

	// iterate over each event.
	while (!events.empty()) {
		auto event = events.top();
		switch (event.Type) {
		case EventType::ChangeScene: {
			auto args = std::get<ChangeSceneArgs>(event.Args);
			if (mScene) {
				mScene->OnExit(*this);
			}
			mScene = mScenes[args.NextSceneName];
			if (mScene) {
				mScene->OnEnter(*this);
			}
			break;
		}
		default:
			break;
		}
		events.pop();
	}
}

void Context::Render(double alpha)
{
	mGraphics->BeginDraw();
	// TODO
	mGraphics->EndDraw();
}

void Context::ChangeScene(const std::string& name)
{
	// just check that we actually have the target scene.
	if (mScenes.find(name) == mScenes.end()) {
		throw "Invalid scene name provided";
	}

	// enqueue an event about the change.
	Event e;
	e.Priority = 0;
	e.Type = EventType::ChangeScene;
	e.Args = ChangeSceneArgs{ name };
	mEvents.push(e);
}

void Context::EnqueueEvent(const Event& event)
{
	mEvents.push(event);
}

void Context::SetWindow(Windows::UI::Core::CoreWindow^ window)
{
	mGraphics->SetWindow(window);
}