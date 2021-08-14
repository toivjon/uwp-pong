#pragma once

#include "vec2f.h"

// A simple 2D axis aligned bounding box implementation.
class AABB {
public:
	AABB(const Vec2f& center, const Vec2f& extent) { this->center = center;  this->extent = extent; }

	void setCenter(const Vec2f& center) { this->center = center; }
	void setExtent(const Vec2f& extent) { this->extent = extent; }

	auto getCenter() const -> const Vec2f& { return center; }
	auto getExtent() const -> const Vec2f& { return extent; }

	auto min(int axis) const -> float { return center[axis] - extent[axis]; }
	auto max(int axis) const -> float { return center[axis] + extent[axis]; }

	auto collides(const AABB& aabb) const -> bool {
		const auto centerDiff = aabb.center - center;
		const auto extentSum = aabb.extent + extent;
		return fabsf(centerDiff.getX()) <= extentSum.getX() && fabsf(centerDiff.getY()) <= (extentSum.getY());
	}
private:
	Vec2f center;
	Vec2f extent;
};
