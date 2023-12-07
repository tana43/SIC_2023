#pragma once
#include "../Graphics/Particles.h"
#include <memory>

class DemoEffect
{
    enum class ParticleData
    {
        Width,
        BaseSpeed
    };

public:

    DemoEffect(size_t particleCount);
    ~DemoEffect() {}

    void Update(float elapsedTime);
    void Render();
    void DrawDebug();

    void Play(DirectX::XMFLOAT3 pos);

    const bool GetIsPlay() const { return isPlay; }

    void SetColor(const DirectX::XMFLOAT4 color) { particles->color = color; }
    const DirectX::XMFLOAT4 GetColor() const { return particles->color; }

    void SetScale(const float sca) { scale = sca; }

private:
    std::unique_ptr<Regal::Graphics::TestParticles> particles;

    float timer;
    float effectTime{2.0f};
    bool isPlay;

    DirectX::XMFLOAT3 pos;
    float scale{0.15f};

    float width = 10;
    float baseSpeed = 2;
};