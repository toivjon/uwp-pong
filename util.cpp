#include "util.h"

#include <cassert>
#include <chrono>
#include <random>

using namespace std::chrono;

long long pong::util::GetCurrentMilliseconds() {
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

int pong::util::GetRandomIntBetween(int min, int max) {
	assert(min <= max);
	static std::random_device rd;
	static std::mt19937 mt(rd());
	std::uniform_int_distribution<int> dist(min, max);
	return dist(mt);
}

float pong::util::ConvertDipsToPixels(float dips, float dpi) {
	assert(dpi > 0.f);
	assert(dips > 0.f);
	static const float dipsPerInch = 96.f;
	return floorf(dips * dpi / dipsPerInch + 0.5f);
}

void pong::util::ThrowIfFailed(HRESULT hresult) {
	if (FAILED(hresult)) {
		throw Platform::Exception::CreateException(hresult);
	}
}