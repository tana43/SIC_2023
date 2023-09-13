#include "AudioManager.h"
#include "RegalLib/Other/Misc.h"

void AudioManager::Initialize()
{
	HRESULT hr{ S_OK };

	// XAUDIO2
	hr = XAudio2Create(xaudio2.GetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = xaudio2->CreateMasteringVoice(&masterVoice);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	bgm = std::make_unique<audio>(xaudio2.Get(), L"./Resources/Audio/Bgm.wav");

	for (int i = 0; i < 10; i++)
	{
		se[SE::ALL_BLOCK_ERASE][i] = std::make_unique<audio>(xaudio2.Get(), L"./Resources/Audio/AllBlockErase.wav");
		se[SE::BLOCK_ASSAULT]  [i] = std::make_unique<audio>(xaudio2.Get(), L"./Resources/Audio/BlockAssault.wav");
		se[SE::CURSOR_MOVE]    [i] = std::make_unique<audio>(xaudio2.Get(), L"./Resources/Audio/CursorMove.wav");
		se[SE::DAMAGED]        [i] = std::make_unique<audio>(xaudio2.Get(), L"./Resources/Audio/Damaged.wav");
		se[SE::DECIDE]         [i] = std::make_unique<audio>(xaudio2.Get(), L"./Resources/Audio/Decide.wav");
		se[SE::DAMAGED]        [i] = std::make_unique<audio>(xaudio2.Get(), L"./Resources/Audio/Damaged.wav");
		se[SE::ENEMY_DIE]      [i] = std::make_unique<audio>(xaudio2.Get(), L"./Resources/Audio/EnemyDie.wav");
		se[SE::LOSE]           [i] = std::make_unique<audio>(xaudio2.Get(), L"./Resources/Audio/Lose.wav");
		se[SE::PUT_BLOCK]      [i] = std::make_unique<audio>(xaudio2.Get(), L"./Resources/Audio/PutBlock.wav");
		se[SE::ROT_BLOCK]      [i] = std::make_unique<audio>(xaudio2.Get(), L"./Resources/Audio/RotBlock.wav");
		se[SE::SHOT]           [i] = std::make_unique<audio>(xaudio2.Get(), L"./Resources/Audio/Shot.wav");
	}
	


	bgm->play(255);
    //bgm.emplace_back(std::make_unique<audio>(xaudio2.Get()));
}

void AudioManager::Play(int seType)
{
	se[seType][seMulti[seType]]->play();
	seMulti[seType]++;
	if (seMulti[seType] > END - 1)
	{
		seMulti[seType] = 0;
	}
}

