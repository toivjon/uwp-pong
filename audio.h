#pragma once

#include <wrl.h>
#include <xaudio2.h>

namespace pong
{
	class Audio final
	{
	public:
		// The constructor is used to create instance of the XAudio2 engine.
		Audio();

		// The destructor handles the graceful release of resources.
		~Audio();
	private:
		// A COM smart pointer reference to XAudio2 sound engine.
		Microsoft::WRL::ComPtr<IXAudio2> mXAudio2;

		// The main master voice used to playback sounds.
		IXAudio2MasteringVoice* mMasterVoice;
	};
}
