#pragma once

#include "./RegalLib/Regal.h"

class GameScene final : public Regal::Scene::BaseScene
{
public:
    GameScene() {}
    ~GameScene()override {}

    void CreateResource()override;
    void Initialize() override;
    void Finalize() override;
    void Begin() override;
    void Update(const float& elapsedTime)override;
    void End() override;
    void Render(const float& elapsedTime)override;

    void DrawDebug()override;
    void PostEffectDrawDebug()override;

private:
    float clearColor[4] = { 0.1f,0.1f,0.1f,1.0f };
    std::unique_ptr<Regal::Graphics::Framebuffer> framebuffer;
    std::unique_ptr<Regal::Graphics::Bloom> bloomer;
    std::unique_ptr<Regal::Graphics::FullscreenQuad> bitBlockTransfer;

    //�P�x�������o�p�V�F�[�_�[
    Microsoft::WRL::ComPtr<ID3D11PixelShader> LEPixelShader;

    std::unique_ptr<Regal::Resource::Sprite> sprite;
};
