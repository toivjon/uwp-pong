#pragma once

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

	struct Hit {
		bool  collided;
		Vec2f position;
		Vec2f delta;
		Vec2f normal;
		float time;
	};

	struct Sweep {
		Hit   hit;
		Vec2f position;
		float time;
	};

	auto intersects(const AABB& other) const -> Hit {
		const auto dx = other.center.getX() - center.getX();
		const auto px = other.extent.getX() + extent.getX() - fabsf(dx);
		if (px <= 0.f) {
			return Hit{ false };
		}
		const auto dy = other.center.getY() - center.getY();
		const auto py = other.extent.getY() + extent.getY() - fabsf(dy);
		if (py >= 0.f) {
			return Hit{ false };
		}
		Hit hit = {};
		hit.delta = { 0.f, 0.f };
		hit.normal = { 0.f, 0.f };
		hit.time = 0.f;
		if (px < py) {
			const auto sx = sign(dx);
			hit.delta.setX(px * sx);
			hit.normal.setX(sx);
			hit.position.setX(center.getX() + extent.getX() * sx);
			hit.position.setY(other.center.getY());
		} else {
			const auto sy = sign(dx);
			hit.delta.setY(py * sy);
			hit.normal.setY(sy);
			hit.position.setX(other.center.getX());
			hit.position.setY(center.getY() + extent.getY() * sy);
		}
		return hit;
	}

	auto intersects(const Vec2f& origin, const Vec2f& delta, const Vec2f& padding) const -> Hit {
		const auto scaleX = 1.f / delta.getX();
		const auto scaleY = 1.f / delta.getY();
		const auto signX = sign(scaleX);
		const auto signY = sign(scaleY);
		const auto nearTimeX = (center.getX() - signX * (extent.getX() + padding.getX())- origin.getX()) * scaleX;
		const auto nearTimeY = (center.getY() - signY * (extent.getY() + padding.getY())- origin.getY()) * scaleY;
		const auto farTimeX = (center.getX() + signX * (extent.getX() + padding.getX()) - origin.getX()) * scaleX;
		const auto farTimeY = (center.getY() + signY * (extent.getY() + padding.getY()) - origin.getY()) * scaleY;
		if (nearTimeX > farTimeX || nearTimeY > farTimeY) {
			return { false };
		}
		const auto nearTime = std::max(nearTimeX, nearTimeY);
		const auto farTime = std::min(farTimeX, farTimeY);
		if (nearTime >= 1.f || farTime <= 0.f) {
			return { false };
		}
		Hit hit = {};
		hit.collided = true;
		hit.time = clamp(nearTime, 0.f, 1.f);
		if (nearTimeX > nearTimeY) {
			hit.normal.setX(-signX);
			hit.normal.setY(0.f);
		} else {
			hit.normal.setX(0.f);
			hit.normal.setY(-signY);
		}
		hit.delta.setX((1.f - hit.time) * -delta.getX());
		hit.delta.setY((1.f - hit.time) * -delta.getY());
		hit.position.setX(origin.getX() + delta.getX() * hit.time);
		hit.position.setY(origin.getY() + delta.getY() * hit.time);
		return hit;
	}

	static auto sweep(const AABB& aabb1, const AABB& aabb2, const Vec2f& delta) -> Sweep {
		static const auto epsilon = 1e-8f;
		if (fabsf(delta.getX()) < epsilon && fabsf(delta.getY()) < epsilon) {
			Sweep sweep = {};
			sweep.hit = aabb1.intersects(aabb2);
			sweep.time = sweep.hit.time;
			sweep.position = sweep.hit.position;
			return sweep;
		}
		Sweep sweep = {};
		sweep.hit = aabb1.intersects(aabb2.getCenter(), delta, aabb2.getExtent());
		if (sweep.hit.collided) {
			sweep.time = clamp(sweep.hit.time, 0.f, 1.f);
			auto direction = delta.normalized();
			sweep.position.setX(clamp(
				sweep.position.getX() + direction.getX() * aabb2.getExtent().getX(),
				aabb1.center.getX() - aabb1.extent.getX(),
				aabb1.center.getX() + aabb1.extent.getX())
			);
			sweep.position.setY(clamp(
				sweep.position.getY() + direction.getY() * aabb2.getExtent().getY(),
				aabb1.center.getY() - aabb1.extent.getY(),
				aabb1.center.getY() + aabb1.extent.getY())
			);
		} else {
			sweep.position.setX(aabb2.center.getX() + v.getX());
			sweep.position.setY(aabb2.center.getY() + v.getY());
			sweep.time = 1.f;
		}
		return sweep;
	}

	static auto sweep(const AABB& aabb1, const AABB& aabb2, const Vec2f& v1, const Vec2f& v2) -> Sweep {
		const auto delta = v1 - v2;
		auto hitResult = sweep(aabb1, aabb2, delta);
		if (hitResult.hit.collided) {
			hitResult.position.setX(aabb1.center.getX() + v1.getX() * hitResult.time);
			hitResult.position.setY(aabb1.center.getY() + v1.getY() * hitResult.time);
			auto position = Vec2f{
				aabb2.center.getX() + v2.getX() * hitResult.time,
				aabb2.center.getY() + v2.getY() * hitResult.time,
			};
			const auto direction = position - hitResult.position;
			const auto amt = direction * .5f;
			hitResult.position.setX(hitResult.position.getX() + amt);
			hitResult.position.setY(hitResult.position.getY() + amt);
		}
		return hitResult;
	}
private:
	Vec2f center;
	Vec2f extent;
};
