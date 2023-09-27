#pragma once
#include "RegalLib/Regal.h"

class BaseColorController
{
private:
    BaseColorController() {}
    ~BaseColorController() {}

public:
    static BaseColorController& Instance()
    {
        static BaseColorController instance;
        return instance;
    }

    void Initialize();
    void Update(float elapsedTime);
    void Render();

    void ChangeColorBGParticle(DirectX::XMFLOAT4 color);

    //Z色をランダムで取得
    static DirectX::XMFLOAT4 GetRundomBrightColor();

    DirectX::XMFLOAT4 baseColor;

    //背景用パーティクル
    std::unique_ptr<Regal::Graphics::Particles> BGParticles;

    //パーティクルの色変更演出フラグ
    bool isBGColorChange;
    float changeTimer;
    float changeMaxTime = 1.5f;

    DirectX::XMFLOAT4 particleColor;
};

