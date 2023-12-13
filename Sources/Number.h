#pragma once
#include "RegalLib/Regal.h"

class Number
{
public:
    Number();
    ~Number() {}

    void Render();

    void SetNumber(int n);

    void SetColor(const DirectX::XMFLOAT4 c) { numSprite->SetColor(c.x, c.y, c.z, c.w); }

    void DrawDebug() {}

public:
    std::unique_ptr<Regal::Resource::Sprite> numSprite;

    int iWidth{60};
    int iHeight{100};

    DirectX::XMFLOAT2 pos;
    DirectX::XMFLOAT2 scale{0.5f,0.5f};
    DirectX::XMFLOAT2 trimmingPos{0,0};
};

class Numbers
{
public:
    Numbers(int n = 0);
    ~Numbers() {}

    void Render();

    void SetNumbers(int n);

    void DrawDebug();

    void SetPosition(const DirectX::XMFLOAT2 pos) { this->pos = pos; }

    void SetScale(const float sca) { scale = sca; }
    void SetBetweenNum(const float bet) { betweenNumbers = bet; }

    void SetAlpha(float alpha) 
    {
        for (int i = 0; i < 10; i++)
        {
            num[i].SetColor(DirectX::XMFLOAT4(1,1,1,alpha));
        }
    }

private:

    DirectX::XMFLOAT2 pos;

    int myNum{0};

    //Œ…”
    int digit{ 1 };

    //Å‘å10Œ…•\Ž¦
    Number num[10];

    //”Žš‚Æ”Žš‚ÌŠÔŠu
    float betweenNumbers{25};

    float scale{0.5f};
};