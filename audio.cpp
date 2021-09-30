#include "pch.h"
#include "audio.hpp"

#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid.lib")

using namespace winrt;

Audio::Audio() {
	check_hresult(XAudio2Create(mEngine.put()));
	check_hresult(mEngine->CreateMasteringVoice(&mMasteringVoice));
	check_hresult(MFStartup(MF_VERSION));
}

Audio::~Audio() {
	MFShutdown();
}

auto Audio::createSound(const std::wstring& filename) -> Sound {
	auto const streamIndex = static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM);

	// Build a media source reader instance.
	com_ptr<IMFSourceReader> reader;
	check_hresult(MFCreateSourceReaderFromURL(filename.c_str(), nullptr, reader.put()));
	check_hresult(reader->SetStreamSelection(static_cast<DWORD>(MF_SOURCE_READER_ALL_STREAMS), false));
	check_hresult(reader->SetStreamSelection(streamIndex, true));

	// Specify read instructions for the reader.
	com_ptr<IMFMediaType> mediaType;
	check_hresult(MFCreateMediaType(mediaType.put()));
	check_hresult(mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));
	check_hresult(mediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM));
	check_hresult(reader->SetCurrentMediaType(streamIndex, 0, mediaType.get()));

	// Get information about the output format of the reader.
	com_ptr<IMFMediaType> outputMediaType;
	check_hresult(reader->GetCurrentMediaType(streamIndex, outputMediaType.put()));

	// Build a new wave format from the reader output format.
	UINT32 size = 0;
	WAVEFORMATEX* waveFormat = nullptr;
	check_hresult(MFCreateWaveFormatExFromMFMediaType(outputMediaType.get(), &waveFormat, &size));

	// Ensure that the target stream is being selected.
	check_hresult(reader->SetStreamSelection(streamIndex, true));

	// Read the data into the sound object.
	BYTE* audioData = nullptr;
	DWORD audioDataSize = 0;
	Sound sound = {};
	sound.ptr = nullptr;
	while (true) {
		com_ptr<IMFSample> sample;
		com_ptr<IMFMediaBuffer> buffer;
		DWORD flags = 0;
		check_hresult(reader->ReadSample(streamIndex, 0, nullptr, &flags, nullptr, sample.put()));

		// check whether data type is changed or EOF has been reached.
		if (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
			break;
		if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
			break;

		// get data from the audio sample via a buffer.
		check_hresult(sample->ConvertToContiguousBuffer(buffer.put()));
		check_hresult(buffer->Lock(&audioData, nullptr, &audioDataSize));
		for (auto i = 0u; i < audioDataSize; i++) {
			sound.bytes.push_back(audioData[i]);
		}
		check_hresult(buffer->Unlock());
	}

	// Create a new sound with a desired sound format.
	check_hresult(mEngine->CreateSourceVoice(&sound.ptr, waveFormat));
	return sound;
}

void Audio::playSound(const Sound& sound) {
	XAUDIO2_BUFFER waveBuffer = {};
	waveBuffer.AudioBytes = static_cast<UINT32>(sound.bytes.size());
	waveBuffer.pAudioData = &sound.bytes[0];
	check_hresult(sound.ptr->SubmitSourceBuffer(&waveBuffer));
	sound.ptr->Start();
}