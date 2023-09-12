#pragma once
#include "RegalLib/Regal.h"

class PopEffect
{
public:
    PopEffect(size_t particleCount);
    ~PopEffect() {}

    void Update(float elapsedTime);
    void Render();
    void DrawDebug();

    void Play(DirectX::XMFLOAT3 pos);

    const bool GetIsPlay() const { return isPlay; }

    void SetColor(const DirectX::XMFLOAT4 color) { popParticle->color = color; }
    const DirectX::XMFLOAT4 GetColor() const { return popParticle->color; }

    void SetScale(const float sca) { scale = sca; }

private:
    std::unique_ptr<Regal::Graphics::PopParticles> popParticle;

    float timer;
    float effectTime{2.0f};
    bool isPlay;

    DirectX::XMFLOAT3 pos;
    float scale{0.15f};
};