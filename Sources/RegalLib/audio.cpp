// XAUDIO2

#include "audio.h"

#include <windows.h>
#include <winerror.h>

HRESULT find_chunk(HANDLE hfile, DWORD fourcc, DWORD& chunk_size, DWORD& chunk_data_position)
{

	HRESULT hr = S_OK;

	if (INVALID_SET_FILE_POINTER == SetFilePointer(hfile, 0, NULL, FILE_BEGIN))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	DWORD chunk_type;
	DWORD chunk_data_size;
	DWORD riff_data_size = 0;
	DWORD file_type;
	DWORD bytes_read = 0;
	DWORD offset = 0;

	while (hr == S_OK)
	{
		DWORD number_of_bytes_read;
		if (0 == ReadFile(hfile, &chunk_type, sizeof(DWORD), &number_of_bytes_read, NULL))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

		if (0 == ReadFile(hfile, &chunk_data_size, sizeof(DWORD), &number_of_bytes_read, NULL))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

		switch (chunk_type)
		{
		case 'FFIR'/*RIFF*/:
			riff_data_size = chunk_data_size;
			chunk_data_size = 4;
			if (0 == ReadFile(hfile, &file_type, sizeof(DWORD), &number_of_bytes_read, NULL))
			{
				hr = HRESULT_FROM_WIN32(GetLastError());
			}
			break;

		default:
			if (INVALID_SET_FILE_POINTER == SetFilePointer(hfile, chunk_data_size, NULL, FILE_CURRENT))
			{
				return HRESULT_FROM_WIN32(GetLastError());
			}
		}

		offset += sizeof(DWORD) * 2;

		if (chunk_type == fourcc)
		{
			chunk_size = chunk_data_size;
			chunk_data_position = offset;
			return S_OK;
		}

		offset += chunk_data_size;

		if (bytes_read >= riff_data_size)
		{
			return S_FALSE;
		}
	}

	return S_OK;

}

HRESULT read_chunk_data(HANDLE hFile, LPVOID buffer, DWORD buffer_size, DWORD buffer_offset)
{
	HRESULT hr = S_OK;
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, buffer_offset, NULL, FILE_BEGIN))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}
	DWORD number_of_bytes_read;
	if (0 == ReadFile(hFile, buffer, buffer_size, &number_of_bytes_read, NULL))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}
	return hr;
}

audio::audio(IXAudio2* xaudio2, const wchar_t* filename)
{
	HRESULT hr;

	// Open the file
	HANDLE hfile = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == hfile)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}

	if (INVALID_SET_FILE_POINTER == SetFilePointer(hfile, 0, NULL, FILE_BEGIN))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}

	DWORD chunk_size;
	DWORD chunk_position;
	//check the file type, should be 'WAVE' or 'XWMA'
	find_chunk(hfile, 'FFIR'/*RIFF*/, chunk_size, chunk_position);
	DWORD filetype;
	read_chunk_data(hfile, &filetype, sizeof(DWORD), chunk_position);
	_ASSERT_EXPR(filetype == 'EVAW'/*WAVE*/, L"Only support 'WAVE'");

	find_chunk(hfile, ' tmf'/*FMT*/, chunk_size, chunk_position);
	read_chunk_data(hfile, &wfx, chunk_size, chunk_position);

	//fill out the audio data buffer with the contents of the fourccDATA chunk
	find_chunk(hfile, 'atad'/*DATA*/, chunk_size, chunk_position);
	BYTE* data = new BYTE[chunk_size];
	read_chunk_data(hfile, data, chunk_size, chunk_position);

	buffer.AudioBytes = chunk_size;  //size of the audio buffer in bytes
	buffer.pAudioData = data;  //buffer containing audio data
	buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

	hr = xaudio2->CreateSourceVoice(&source_voice, (WAVEFORMATEX*)&wfx);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

audio::~audio()
{
	source_voice->DestroyVoice();
	delete[] buffer.pAudioData;
}
void audio::play(int loop_count)
{
	HRESULT hr;

	XAUDIO2_VOICE_STATE voice_state = {};
	source_voice->GetState(&voice_state);

	if (voice_state.BuffersQueued)
	{

		//stop(false, 0);
		return;
	}

	buffer.LoopCount = loop_count;
	hr = source_voice->SubmitSourceBuffer(&buffer);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = source_voice->Start(0);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}
void audio::stop(bool play_tails/*Continue emitting effect output after the voice is stopped. */, bool wait_for_buffer_to_unqueue)
{
	XAUDIO2_VOICE_STATE voice_state = {};
	source_voice->GetState(&voice_state);
	if (!voice_state.BuffersQueued)
	{
		return;
	}

	HRESULT hr;
	hr = source_voice->Stop(play_tails ? XAUDIO2_PLAY_TAILS : 0);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = source_voice->FlushSourceBuffers();
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	while (wait_for_buffer_to_unqueue && voice_state.BuffersQueued)
	{
		source_voice->GetState(&voice_state);
	}
}

void audio::volume(float volume)
{
	HRESULT hr = source_voice->SetVolume(volume);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

bool audio::queuing()
{
	XAUDIO2_VOICE_STATE voice_state = {};
	source_voice->GetState(&voice_state);
	return voice_state.BuffersQueued;
}