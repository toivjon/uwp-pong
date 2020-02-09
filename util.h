#pragma once

#include <chrono>
#include <winerror.h>

namespace pong
{
	// A utility function to throw an exception if HRESULT was failed.
	inline void ThrowIfFailed(HRESULT hr) {
		if (FAILED(hr)) {
			throw Platform::Exception::CreateException(hr);
		}
	}

	// A utility function to get current time in milliseconds.
	inline unsigned long CurrentMillis() {
		using namespace std::chrono;
		return (unsigned long)duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	}
}
