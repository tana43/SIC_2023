#include "Fade.h"

void Fade::CreateResource()
{
    sTitle_0 = std::make_unique<Regal::Resource::Sprite>(Regal::Graphics::Graphics::Instance().GetDevice(),
        L"./Resources/Images/BlackScreen.png");
}

void Fade::Initialize()
{
    
    sTitle_0->SetColor(1, 1, 1, 1);

    AutoFadeIn();
}

void Fade::Update(float elapsedTime)
{
    if (active)
    {
        if (isFadeIn)
        {
            //フェードイン
            alpha -= elapsedTime * fadeSpeed;
            sTitle_0->SetAlpha(alpha);

            if (alpha <= 0)
            {
                active = false;
                hideScereen = false;
            }

        }
        else
        {
            //フェードアウト
            alpha += elapsedTime * fadeSpeed;
            sTitle_0->SetAlpha(alpha);

            if (alpha >= 1)
            {
                active = false;
                hideScereen = true;
            }
        }
    }
}

void Fade::Render(ID3D11DeviceContext* immediateContext)
{
    sTitle_0->Render();
}

bool Fade::FadeIn(float elapsedTime)
{
    if (!hideScereen)return true;

    alpha -= elapsedTime * fadeSpeed;
    sTitle_0->SetAlpha(alpha);

    if (alpha <= 0)
    {
        hideScereen = false;
        return true;
    }

    return false;
}

bool Fade::FadeOut(float elapsedTime)
{
    if (hideScereen)return true;

    alpha += elapsedTime * fadeSpeed;
    sTitle_0->SetAlpha(alpha);

    if (alpha >= 1)
    {
        hideScereen = true;
        return true;
    }

    return false;
}

void Fade::AutoFadeIn()
{
    active = true;

    isFadeIn = true;
}

void Fade::AutoFadeOut()
{
    active = true;

    isFadeIn = false;
}
