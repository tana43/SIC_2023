#pragma once

#include "./RegalLib/Regal.h"
#include "PopEffect.h"

class TitleScene final : public Regal::Scene::BaseScene
{
public:
    TitleScene() {}
    ~TitleScene()override {}

    void CreateResource()override;
    void Initialize() override;
    void Finalize() override;
    void Begin() override;
    void Update(const float& elapsedTime)override;
    void End() override;
    void Render(const float& elapsedTime)override;

    void DrawDebug()override;
    void PostEffectDrawDebug()override;

    enum SelectMenu
    {
        PLAY,
        TUTORIAL,
        EXIT,
    };

private:
    float clearColor[4] = { 0.0f,0.0f,0.0f,1.0f };
    std::unique_ptr<Regal::Graphics::Framebuffer> framebuffer;
    std::unique_ptr<Regal::Graphics::Bloom> bloomer;
    std::unique_ptr<Regal::Graphics::FullscreenQuad> bitBlockTransfer;

    //輝度成分抽出用シェーダー
    Microsoft::WRL::ComPtr<ID3D11PixelShader> LEPixelShader;

    std::unique_ptr<Regal::Resource::Sprite> sprite;
    std::unique_ptr<Regal::Resource::Sprite> sCursor;
    std::unique_ptr<Regal::Resource::Sprite> sTutrial;
    DirectX::XMFLOAT2 sCursorPos{512, 405};
    //DirectX::XMFLOAT2 sPos;

    int cursorState{PLAY};

    std::unique_ptr<Regal::Graphics::Particles> BGParticles;
    std::unique_ptr<PopEffect> popEffect;

    bool isDecide{false};
    bool once{false};
    bool openTutrial{ false };
};

