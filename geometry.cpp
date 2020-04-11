#include "geometry.h"

using namespace pong::geometry;

bool Rectangle::Contains(float x, float y) const {
	return left <= x && x <= right && top <= y && y <= bottom;
}

bool Rectangle::Contains(const Rectangle& rect) const {
	return Contains(rect.left, rect.top)
		&& Contains(rect.left, rect.bottom)
		&& Contains(rect.right, rect.top)
		&& Contains(rect.right, rect.bottom);
}

bool Rectangle::Collides(const Rectangle& rect) const {
	return !(right < rect.left || bottom < rect.top || left > rect.right || top > rect.bottom);
}

void Rectangle::Set(float left, float top, float right, float bottom) {
	this->left = left;
	this->top = top;
	this->right = right;
	this->bottom = bottom;
}

void Rectangle::Move(float x, float y) {
	left += x;
	right += x;
	top += y;
	bottom += y;
}