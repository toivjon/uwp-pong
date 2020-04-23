#pragma once

#include <winerror.h>

namespace pong::util
{
	long long GetCurrentMilliseconds();
	int GetRandomIntBetween(int min, int max);
	float ConvertDipsToPixels(float dips, float dpi);
	void ThrowIfFailed(HRESULT hresult);
}
