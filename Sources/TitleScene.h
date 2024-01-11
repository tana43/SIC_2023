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

    //タイトル画面とメインメニュー画面の切り替え処理
    void MenuUpdate(float elapsedTime);

    void MainMenuUpdate(float elapsedTime);

    void TransitionMainMenuState();

    enum class SubScene
    {
        TITLE,
        MAIN_MENU
    };

    enum class SelectMenu
    {
        PLAY,
        PRACTICE,
        EXIT,
    };

public:
    Regal::Resource::Sprite* GetSprite_Play() { return sTitle_play.get(); }
    Regal::Resource::Sprite* GetSprite_Practice() { return sTitle_practice.get(); }
    Regal::Resource::Sprite* GetSprite_BackToTitle() { return sTitle_backToTitle.get(); }

    const float GetFadeColorTimer() const { return fadeColorTimer; }

private:
    float clearColor[4] = { 0.0f,0.0f,0.0f,1.0f };
    std::unique_ptr<Regal::Graphics::Framebuffer> framebuffer;
    std::unique_ptr<Regal::Graphics::Bloom> bloomer;
    std::unique_ptr<Regal::Graphics::FullscreenQuad> bitBlockTransfer;

    //輝度成分抽出用シェーダー
    Microsoft::WRL::ComPtr<ID3D11PixelShader> LEPixelShader;

    std::unique_ptr<Regal::Resource::Sprite> sTitle_0;
    std::unique_ptr<Regal::Resource::Sprite> sTitle_A;
    std::unique_ptr<Regal::Resource::Sprite> sTitle_Enter;
    std::unique_ptr<Regal::Resource::Sprite> sTitle_play;
    std::unique_ptr<Regal::Resource::Sprite> sTitle_practice;
    std::unique_ptr<Regal::Resource::Sprite> sTitle_backToTitle;
    //std::unique_ptr<Regal::Resource::Sprite> sCursor;
    std::unique_ptr<Regal::Resource::Sprite> sTutrialGamePad;
    std::unique_ptr<Regal::Resource::Sprite> sTutrialKeyboard;
    DirectX::XMFLOAT2 sCursorPos{512, 405};
    //DirectX::XMFLOAT2 sPos;

    DirectX::XMFLOAT4 rundomColor;//初期化毎にランダムで生成されるカラー

    int cursorState{ static_cast<int>(SelectMenu::PLAY)};

    std::unique_ptr<Regal::Graphics::Particles> BGParticles;
    std::unique_ptr<PopEffect> popEffect;

    bool isDecide{false};
    bool once{false};
    bool openTutrial{ false };

    int state;

    float timer;

    float slideParameter;//0~1の値で管理し、各メニュー文字のスライドをさせる

    bool isChangeTime;//透明度とか変更中
    bool isMenuProceed;//選択された項目へ進んでいいタイミングか？

    float fadeColorTimer;
};

