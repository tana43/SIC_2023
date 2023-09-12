#pragma once
#include "RegalLib/Regal.h"

class Number
{
public:
    Number() 
    {
        sprite = std::make_unique<Regal::Resource::Sprite>(
            Regal::Graphics::Graphics::Instance().GetDevice()
            , L"./Resources/Images/numbers.png");
    }
    ~Number() {}

    void Render();

    void SetNumber(int n);

    void SetColor(const DirectX::XMFLOAT4 c) { sprite->SetColor(c.x, c.y, c.z, c.w); }

    void DrawDebug() {}

public:
    std::unique_ptr<Regal::Resource::Sprite> sprite;

    int iWidth{60};
    int iHeight{100};

    DirectX::XMFLOAT2 pos;
    DirectX::XMFLOAT2 scale{0.5f,0.5f};
    DirectX::XMFLOAT2 trimmingPos{0,0};
};

class Numbers
{
public:
    Numbers(int n = 0):myNum(n) 
    {
        int num{ myNum };
        int d{};
        while (num != 0)
        {
            num /= 10;

            ++d;
        }
        digit = d;
    }
    ~Numbers() {}

    void Render();

    void SetNumbers(int n);

    void DrawDebug();

    void SetPosition(const DirectX::XMFLOAT2 pos) { this->pos = pos; }

private:

    DirectX::XMFLOAT2 pos;

    int myNum{0};

    //Œ…”
    int digit{ 1 };

    //Å‘å10Œ…•\¦
    Number num[10];

    //”š‚Æ”š‚ÌŠÔŠu
    float betweenNumbers{25};
};