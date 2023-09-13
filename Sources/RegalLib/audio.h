#pragma once
// XAUDIO2

#include <xaudio2.h>
#include <mmreg.h>

#include "./Other/Misc.h"

class audio
{
	WAVEFORMATEXTENSIBLE wfx = { 0 };
	XAUDIO2_BUFFER buffer = { 0 };

	IXAudio2SourceVoice* source_voice;

public:
	audio(IXAudio2* xaudio2, const wchar_t* filename);
	virtual ~audio();
	void play(int loop_count = 0/*255 : XAUDIO2_LOOP_INFINITE*/);
	void stop(bool play_tails = true, bool wait_for_buffer_to_unqueue = true);
	void volume(float volume);
	bool queuing();
};

