#include "PopEffect.h"

PopEffect::PopEffect(size_t particleCount)
{
    popParticle = std::make_unique<Regal::Graphics::PopParticles>(
        Regal::Graphics::Graphics::Instance().GetDevice(), particleCount
    );
}

void PopEffect::Update(float elapsedTime)
{
    if (isPlay)
    {
        if (timer > effectTime)isPlay = false;

        popParticle->particleData.particleSize = ((effectTime - timer) / effectTime) * scale;

        popParticle->Integrate(Regal::Graphics::Graphics::Instance().GetDeviceContext(), elapsedTime);

        timer += elapsedTime;
    }
}

void PopEffect::Render()
{
    if (!isPlay)return;
    using namespace Regal::Graphics;
    auto& graphics{ Graphics::Instance() };
    graphics.SetStates(ZT_ON_ZW_ON,CULL_NONE,ALPHA);
    popParticle->Render(graphics.GetDeviceContext());
    graphics.Set3DStates();
}

void PopEffect::DrawDebug()
{
    static DirectX::XMFLOAT3 pos;
    ImGui::DragFloat3("Position", &pos.x, 0.1f);
    if (ImGui::Button("Play Effect"))Play(pos);
    popParticle->DrawDebug();
}

void PopEffect::Play(DirectX::XMFLOAT3 pos)
{
    popParticle->particleData.emitterPosition = pos;

    popParticle->Initialize(Regal::Graphics::Graphics::Instance().GetDeviceContext(),0);

    isPlay = true;
    timer = 0;
}
