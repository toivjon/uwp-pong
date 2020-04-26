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

bool Entity::Collides(const D2D_RECT_F& rect) const {
	return !(mRect.right < rect.left
		|| mRect.bottom < rect.top
		|| mRect.left > rect.right
		|| mRect.top > rect.bottom);
}

bool Entity::Contains(const D2D_RECT_F& rect) const {
	return Contains(rect.left, rect.top)
		&& Contains(rect.left, rect.bottom)
		&& Contains(rect.right, rect.top)
		&& Contains(rect.right, rect.bottom);
}

bool Entity::Contains(float x, float y) const {
	return mRect.left <= x && x <= mRect.right && mRect.top <= y && y <= mRect.bottom;
}