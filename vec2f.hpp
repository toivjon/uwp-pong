#pragma once

// Vec2f represents a 2D vector with floating point values.
struct Vec2f final {
	auto operator+(const Vec2f& v) const -> Vec2f { return { x + v.x, y + v.y }; }
	auto operator-(const Vec2f& v) const -> Vec2f { return { x - v.x, y - v.y }; }

	void operator+=(const Vec2f& v) { x += v.x; y += v.y; }
	void operator-=(const Vec2f& v) { x -= v.x; y -= v.y; }

	auto operator*(float s) const -> Vec2f { return { x * s, y * s }; }

	auto operator[](int index) const -> float { return index == 0 ? x : y; }

	float x;
	float y;
};