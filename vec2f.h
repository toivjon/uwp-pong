#pragma once

#include <array>

// Vec2f represents a 2D vector with floating point values.
class Vec2f final {
public:
	Vec2f() : Vec2f(0.f, 0.f) {};
	Vec2f(float x, float y) { data = { x, y }; }
	Vec2f(const std::array<float, 2>& data) { this->data = data; }

	void setX(float x) { data[0] = x; }
	void setY(float y) { data[1] = y; }
	void set(float x, float y) { data[0] = x; data[1] = y; }

	auto getX() const -> float { return data[0]; }
	auto getY() const -> float { return data[1]; }

	auto operator+(const Vec2f& v) const -> Vec2f { return { data[0] + v.data[0], data[1] + v.data[1] }; }
	auto operator-(const Vec2f& v) const -> Vec2f { return { data[0] - v.data[0], data[1] - v.data[1] }; }

	void operator+=(const Vec2f& v) { data[0] += v.data[0]; data[1] += v.data[1]; }
	void operator-=(const Vec2f& v) { data[0] -= v.data[0]; data[1] -= v.data[1]; }

	auto operator*(float s) const -> Vec2f { return { data[0] * s, data[1] * s }; }
	auto operator/(float s) const -> Vec2f { return { data[0] / s, data[1] / s }; }

	auto operator[](int index) -> float& { return data[index]; }
	auto operator[](int index) const -> float { return data[index]; }

	auto length() const -> float { return sqrtf(data[0] * data[0] + data[1] * data[1]); }
	auto normalized() const -> Vec2f { auto l = length();  return { data[0] / l, data[1] / l}; }

	static auto lerp(const Vec2f& v0, const Vec2f& v1, float t) -> Vec2f {
		return v0 + (v1 - v0) * t;
	}
private:
	std::array<float, 2> data;
};