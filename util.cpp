#include "util.h"

#include <chrono>
#include <random>

using namespace std::chrono;

long long pong::util::GetCurrentMilliseconds() {
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

int pong::util::GetRandomIntBetween(int min, int max) {
	static std::random_device rd;
	static std::mt19937 mt(rd());
	std::uniform_int_distribution<int> dist(min, max);
	return dist(mt);
}

float pong::util::ConvertDipsToPixels(float dips, float dpi) {
	static const float dipsPerInch = 96.f;
	return floorf(dips * dpi / dipsPerInch + 0.5f);
}