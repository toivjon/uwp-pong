#include "context.h"

using namespace pong;

void Context::Update(unsigned long dt)
{
	// get events from the current queue.
	std::priority_queue<Event> events;
	events.swap(mEvents);

	// iterate over each event.
	while (!events.empty()) {
		auto event = events.top();
		switch (event.Type) {
		case EventType::ChangeScene:
			break;
		default:
			break;
		}
		events.pop();
	}
}

void Context::Render(double alpha)
{
	// TODO
}

void Context::ChangeScene(const std::string& name)
{
	Event e;
	e.Priority = 0;
	e.Type = EventType::ChangeScene;
	e.Args = ChangeSceneArgs{ name };
	mEvents.push(e);
}