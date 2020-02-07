#include "audio.h"
#include "util.h"

using namespace pong;

Audio::Audio()
{
	ThrowIfFailed(XAudio2Create(&mXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR));
	ThrowIfFailed(mXAudio2->CreateMasteringVoice(&mMasterVoice));
}
