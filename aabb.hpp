#pragma once

#include "vec2f.hpp"

// A simple 2D axis aligned bounding box implementation.
class AABB {
public:
	AABB(const Vec2f& center, const Vec2f& extent) {
		min = center - extent;
		max = center + extent;
	}

	struct Intersection {
		bool  collides;
		float time;
	};

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

		// Find the first and last contact from x-axis.
		if (v.x < .0f) {
			if (b.max.x < a.min.x) return intersection;
			if (a.max.x < b.min.x) tmin = std::max((a.max.x - b.min.x) / v.x, tmin);
			if (b.max.x > a.min.x) tmax = std::min((a.min.x - b.max.x) / v.x, tmax);
		}
		if (v.x > .0f) {
			if (b.min.x > a.max.x) return intersection;
			if (b.max.x < a.min.x) tmin = std::max((a.min.x - b.max.x) / v.x, tmin);
			if (a.max.x > b.min.x) tmax = std::min((a.max.x - b.min.x) / v.x, tmax);
		}
		if (tmin > tmax) return intersection;

		// Find the first and last contact from y-axis.
		if (v.y < .0f) {
			if (b.max.y < a.min.y) return intersection;
			if (a.max.y < b.min.y) tmin = std::max((a.max.y - b.min.y) / v.y, tmin);
			if (b.max.y > a.min.y) tmax = std::min((a.min.y - b.max.y) / v.y, tmax);
		}
		if (v.y > .0f) {
			if (b.min.y > a.max.y) return intersection;
			if (b.max.y < a.min.y) tmin = std::max((a.min.y - b.max.y) / v.y, tmin);
			if (a.max.y > b.min.y) tmax = std::min((a.max.y - b.min.y) / v.y, tmax);
		}
		if (tmin > tmax) return intersection;

		intersection.collides = (tmin >= 0.f && tmin <= 1.f);
		intersection.time = tmin;
		return intersection;
	}

	Vec2f min;
	Vec2f max;
};
