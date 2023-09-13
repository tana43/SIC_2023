#pragma once

#include "./RegalLib/Regal.h"
#include "Block.h"
#include "BlockGroup.h"

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
    float clearColor[4] = { 0.0f,0.0f,0.0f,1.0f };
    std::unique_ptr<Regal::Graphics::Framebuffer> framebuffer;
    std::unique_ptr<Regal::Graphics::Bloom> bloomer;
    std::unique_ptr<Regal::Graphics::FullscreenQuad> bitBlockTransfer;

    //輝度成分抽出用シェーダー
    Microsoft::WRL::ComPtr<ID3D11PixelShader> LEPixelShader;

#if _DEBUG
    std::unique_ptr<Regal::Resource::Sprite> sprite;
#endif // _DEBUG

    std::unique_ptr<Regal::Resource::Sprite> sHealth;
    std::unique_ptr<Regal::Resource::Sprite> sEnemy;

    BlockGroup blockGroup;

    //Hexagon hexagon;

    //背景用パーティクル
    std::unique_ptr<Regal::Graphics::Particles> BGParticles;

};

