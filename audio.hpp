#pragma once

#include <memory>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <winrt/base.h>
#include <xaudio2.h>

class Audio final {
public:
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

	auto createSound(const std::wstring& filename) const -> Sound;
private:
	winrt::com_ptr<IXAudio2> engine;
	IXAudio2MasteringVoice*  masteringVoice;
};
