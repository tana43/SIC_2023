#pragma once

#include <vector>
#include <memory>
#include "RegalLib/Regal.h"

class AudioManager
{
private:
    AudioManager() {}
    ~AudioManager() {}

public:
    static AudioManager& Instance()
    {
        static AudioManager instance;
        return instance;
    }

    enum SE
    {
        BLOCK_ASSAULT,
        SHOT,
        ENEMY_DIE,
        LOSE,
        ROT_BLOCK,
        PUT_BLOCK,
        CURSOR_MOVE,
        DECIDE,
        DAMAGED,
        ALL_BLOCK_ERASE,
        END,
    };

    void Initialize();

    //audio* GetSE(int seType) { return se[seType].get(); }

    void Play(int seType);

    IXAudio2* GetXAudio2() { return xaudio2.Get(); }

private:
    // XAUDIO2
    Microsoft::WRL::ComPtr<IXAudio2> xaudio2;
    IXAudio2MasteringVoice* masterVoice = nullptr;
    std::unique_ptr<audio> bgm;
    std::unique_ptr<audio> se[END][10];
    int seMulti[END];
};

