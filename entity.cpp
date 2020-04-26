#include "entity.h"

#include <cassert>

using namespace pong;

void Entity::SetPosition(float x, float y) {
	auto width = mRect.right - mRect.left;
	auto height = mRect.bottom - mRect.top;

	mRect.left = x;
	mRect.top = y;
	mRect.right = x + width;
	mRect.bottom = y + height;
}

void Entity::SetSize(float width, float height) {
	assert(width >= 0.f);
	assert(height >= 0.f);

	mRect.right = mRect.left + width;
	mRect.bottom = mRect.top + height;
}

