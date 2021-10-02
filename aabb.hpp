#pragma once

#include <algorithm>
#include "vec2f.hpp"

// A simple 2D axis aligned bounding box implementation.
class AABB {
public:
	AABB(const Vec2f& center, const Vec2f& extent) {
		min = center - extent;
		max = center + extent;
	}

	// TODO Perhaps we should use a bit more simpler solution for this?
	auto operator+(const AABB& aabb) const -> AABB {
		const auto xmin = std::min(min.x, aabb.min.x);
		const auto ymin = std::min(min.y, aabb.min.y);
		const auto xmax = std::max(max.x, aabb.max.x);
		const auto ymax = std::max(max.y, aabb.max.y);
		auto result = AABB{ {},{} };
		result.min = { xmin, ymin };
		result.max = { xmax, ymax };
		return result;
	}

	struct Intersection {
		bool  collides;
		float time;
	};

	auto getMin(int axis) const -> float { return axis == 0 ? min.x : min.y; }
	auto getMax(int axis) const -> float { return axis == 0 ? max.x : max.y; }

	static auto intersect(const AABB& a, const AABB& b) -> bool {
		return a.min.x <= b.max.x && a.max.x >= b.min.x && a.min.y <= b.max.y && a.max.y >= b.min.y;
	}

	// This algorithm is derived from the RTCD book.
	static auto intersect(const AABB& a, const AABB& b, const Vec2f& va, const Vec2f& vb) -> Intersection {
		Intersection intersection = {};
		intersection.collides = false;
		intersection.time = 0.f;

		// Exit early whether boxes initially collide.
		if (intersect(a, b)) {
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
				if (a.getMax(i) < b.getMin(i)) tmin = std::max((a.getMax(i) - b.getMin(i)) / v[i], tmin);
				if (b.getMax(i) > a.getMin(i)) tmax = std::min((a.getMin(i) - b.getMax(i)) / v[i], tmax);
			}
			if (v[i] > .0f) {
				if (b.getMin(i) > a.getMax(i)) return intersection;
				if (b.getMax(i) < a.getMin(i)) tmin = std::max((a.getMin(i) - b.getMax(i)) / v[i], tmin);
				if (a.getMax(i) > b.getMin(i)) tmax = std::min((a.getMax(i) - b.getMin(i)) / v[i], tmax);
			}
			if (tmin > tmax) return intersection;
		}
		intersection.collides = (tmin <= 1.f);
		intersection.time = tmin;
		return intersection;
	}

	Vec2f min;
	Vec2f max;
};
