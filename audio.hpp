#pragma once

#include <memory>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <winrt/base.h>
#include <xaudio2.h>

class Audio final {
public:
	using Ptr = std::unique_ptr<Audio>;

	struct Sound {
		IXAudio2SourceVoice* ptr = nullptr;
		std::vector<BYTE>    bytes;

		void play() {
			XAUDIO2_BUFFER waveBuffer = {};
			waveBuffer.AudioBytes = static_cast<UINT32>(bytes.size());
			waveBuffer.pAudioData = &bytes[0];
			winrt::check_hresult(ptr->SubmitSourceBuffer(&waveBuffer));
			ptr->Start();
		}
	};

	Audio();
	~Audio();

	auto createSound(const std::wstring& filename) -> Sound;
	void playSound(const Sound& sound);
private:
	winrt::com_ptr<IXAudio2> mEngine;
	IXAudio2MasteringVoice*  mMasteringVoice;
};
