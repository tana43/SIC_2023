#include "DemoEffect.h"
#include "../Graphics/Graphics.h"

#include "../../../External/imgui/imgui.h"

DemoEffect::DemoEffect(size_t particleCount)
{
    particles = std::make_unique<Regal::Graphics::TestParticles>(
        Regal::Graphics::Graphics::Instance().GetDevice(), particleCount
    );
}

void DemoEffect::Update(float elapsedTime)
{
    if (isPlay)
    {
        if (timer > effectTime)isPlay = false;

        particles->particleData.particleSize = ((effectTime - timer) / effectTime) * scale;

        particles->Integrate(Regal::Graphics::Graphics::Instance().GetDeviceContext(), elapsedTime);

        timer += elapsedTime;
    }

    particles->particleData.other[static_cast<int>(ParticleData::Width)] = width;
    particles->particleData.other[static_cast<int>(ParticleData::BaseSpeed)] = baseSpeed;
}

void DemoEffect::Render()
{
    if (!isPlay)return;
    using namespace Regal::Graphics;
    auto& graphics{ Graphics::Instance() };
    graphics.SetStates(ZT_ON_ZW_ON,CULL_NONE,ALPHA);
    particles->Render(graphics.GetDeviceContext());
    graphics.Set3DStates();
}

void DemoEffect::DrawDebug()
{
    if(ImGui::BeginMenu("DemoEffect"))
    {
        ImGui::Begin("DemoEffect");

        static DirectX::XMFLOAT3 pos;
        ImGui::DragFloat3("Position", &pos.x, 0.1f);

        if (ImGui::Button("Play Effect"))Play(pos);

        ImGui::DragFloat("Width", &width, 0.05f, 0);
        ImGui::DragFloat("BaseSpeed", &baseSpeed, 0.05f, 0);

        particles->DrawDebug();

        ImGui::End();
        ImGui::EndMenu();
    }
    
}

void DemoEffect::Play(DirectX::XMFLOAT3 pos)
{
    particles->particleData.emitterPosition = pos;

    particles->Initialize(Regal::Graphics::Graphics::Instance().GetDeviceContext(),0);

    isPlay = true;
    timer = 0;
}
