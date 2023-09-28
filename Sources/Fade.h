#pragma once
#include "RegalLib/Regal.h"

class Fade
{
private:
    Fade()
    {
        CreateResource();
    }
    ~Fade() {}

public:

    void CreateResource();
    void Initialize();
    void Update(float elapsedTime);
    void Render(ID3D11DeviceContext* immediateContext);

    //ここで言うフェードインはゲーム画面が見えていくこと
    bool FadeIn(float elapsedTime);
    bool FadeOut(float elapsedTime);

    //一回呼ぶだけで最後までフェードする
    void AutoFadeIn();
    void AutoFadeOut();

    void SetAlpha(const float alpha) { this->alpha = alpha; }

    
    static Fade& Instance()
    {
        static Fade instance;
        return instance;
    }

private:
    std::unique_ptr<Regal::Resource::Sprite> sTitle_0;
    bool hideScereen{true};
    float alpha{1.0f};
    float fadeSpeed{ 0.4f };

    bool active;
    bool isFadeIn;
};

