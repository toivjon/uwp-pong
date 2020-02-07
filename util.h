#pragma once

#include <winerror.h>

namespace pong
{
	// A utility function to throw an exception if HRESULT was failed.
	inline void ThrowIfFailed(HRESULT hr) {
		if (FAILED(hr)) {
			throw Platform::Exception::CreateException(hr);
		}
	}
}
