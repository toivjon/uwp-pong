#pragma once

#include <algorithm>
#include "vec2f.h"

inline auto sign(float val) -> float {
	return val >= 0.f ? 1.f : -1.f;
}

inline auto clamp(float val, float min, float max) -> float {
	return val < min ? min : val > max ? max : val;
}

// A simple 2D axis aligned bounding box implementation.
class AABB {
public:
	AABB(const Vec2f& center, const Vec2f& extent) { this->center = center;  this->extent = extent; }

	// TODO Perhaps we should use a bit more simpler solution for this?
	auto operator+(const AABB& aabb) const -> AABB {
		const auto xmin = std::min(getMinX(), aabb.getMinX());
		const auto ymin = std::min(getMinY(), aabb.getMinY());
		const auto xmax = std::max(getMaxX(), aabb.getMaxX());
		const auto ymax = std::max(getMaxY(), aabb.getMaxY());
		const auto e = Vec2f{ (xmax - xmin) / 2.f, (ymax - ymin) / 2.f };
		const auto c = Vec2f{ xmin + extent.x, ymin + extent.y };
		return AABB(c, e);
	}

	auto getMinX() const -> float { return center.x - extent.x; }
	auto getMinY() const -> float { return center.y - extent.y; }

	auto getMaxX() const -> float { return center.x + extent.x; }
	auto getMaxY() const -> float { return center.y + extent.y; }

	auto collides(const AABB& aabb) const -> bool {
		const auto centerDiff = aabb.center - center;
		const auto extentSum = aabb.extent + extent;
		return fabsf(centerDiff.x) <= extentSum.x && fabsf(centerDiff.y) <= (extentSum.y);
	}

	struct Intersection {
		bool  collides;
		float time;
	};

	auto getMin(int axis) const -> float { return center[axis] - extent[axis]; }
	auto getMax(int axis) const -> float { return center[axis] + extent[axis]; }

	// This algorithm is derived from the RTCD book.
	static auto intersect(const AABB& a, const AABB& b, const Vec2f& va, const Vec2f& vb) -> Intersection {
		Intersection intersection = {};
		intersection.collides = false;
		intersection.time = 0.f;

		// Exit early whether boxes initially collide.
		if (a.collides(b)) {
			intersection.collides = true;
			intersection.time = 0.f;
			return intersection;
		}

		// We will use relative velocity where 'a' is treated as stationary.
		const auto v = vb - va;

		// Initialize times for the first and last contact.
		auto tmin = -FLT_MAX;
		auto tmax = FLT_MAX;

		// Find first and last contact from each axis.
		for (auto i = 0; i < 2; i++) {
			if (v[i] < .0f) {
				if (b.getMax(i) < a.getMin(i)) return intersection;
				if (a.getMax(i) < b.getMin(i)) {
					auto d = (a.getMax(i) - b.getMin(i)) / v[i];
					if (d > tmin) {
						tmin = d;
					}
				}
				if (b.getMax(i) > a.getMin(i)) tmax = std::min((a.getMin(i) - b.getMax(i)) / v[i], tmax);
			}
			if (v[i] > .0f) {
				if (b.getMin(i) > a.getMax(i)) return intersection;
				if (b.getMax(i) < a.getMin(i)) {
					auto d = (a.getMin(i) - b.getMax(i)) / v[i];
					if (d > tmin) {
						tmin = d;
					}
				}
				if (a.getMax(i) > b.getMin(i)) tmax = std::min((a.getMax(i) - b.getMin(i)) / v[i], tmax);
			}
			if (tmin > tmax) return intersection;
		}
		intersection.collides = true;
		intersection.time = tmin;
		return intersection;
	}
private:
	Vec2f center;
	Vec2f extent;
};
