#include "Number.h"

Number::Number()
{
    sprite = std::make_unique<Regal::Resource::Sprite>(
        Regal::Graphics::Graphics::Instance().GetDevice()
        , L"./Resources/Images/numbers.png");
}

void Number::Render()
{
    auto& graphics{ Regal::Graphics::Graphics::Instance() };

    DirectX::XMFLOAT2 screenCorrection{graphics.GetScreenWidth() / 1280.0f, graphics.GetScreenHeight() / 720.0f};
    sprite->_Render(graphics.GetDeviceContext(), pos.x * screenCorrection.x, pos.y * screenCorrection.y,
        iWidth * screenCorrection.x * scale.x,iHeight * screenCorrection.y * scale.y,
        trimmingPos.x,trimmingPos.y,
        iWidth,iHeight,0);
}

void Number::SetNumber(int n)
{
    trimmingPos.x = n * iWidth;
}

Numbers::Numbers(int n) :myNum(n)
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

void Numbers::Render()
{
    for (int i = 0; i < digit; i++)
    {
        num[i].pos = { pos.x - betweenNumbers * i,pos.y };
        num[i].scale = { scale,scale };
        num[i].Render();
    }
}

void Numbers::SetNumbers(int n)
{
    int nu{ n };
    int d{0};
    while (nu != 0)
    {
        nu /= 10;

        ++d;
    }
    digit = d;
    if (digit == 0)digit = 1;

    for (int i = 0; i < digit; i++)
    {
        num[i].SetNumber(n % 10);
        n /= 10;
    }
}

void Numbers::DrawDebug()
{
    if (ImGui::BeginMenu("Numbers"))
    {
        static int i{ 0 };
        ImGui::DragInt("Num", &i, 1.0f, 0);
        if (ImGui::Button("SetNum"))
        {
            SetNumbers(i);
        }
        ImGui::DragFloat2("Pos", &pos.x);

        ImGui::DragFloat("Scale", &scale,0.01f);

        ImGui::EndMenu();
    }
}
