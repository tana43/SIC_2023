#include "Fade.h"

void Fade::CreateResource()
{
    sprite = std::make_unique<Regal::Resource::Sprite>(Regal::Graphics::Graphics::Instance().GetDevice(),
        L"./Resources/Images/BlackScreen.png");
}

void Fade::Initialize()
{
    
    sprite->SetColor(1, 1, 1, 1);

    AutoFadeIn();
}

void Fade::Update(float elapsedTime)
{
    if (active)
    {
        if (isFadeIn)
        {
            //�t�F�[�h�C��
            alpha -= elapsedTime * fadeSpeed;
            sprite->color[4] = alpha;

            if (alpha <= 0)
            {
                active = false;
                hideScereen = false;
            }

        }
        else
        {
            //�t�F�[�h�A�E�g
            alpha += elapsedTime * fadeSpeed;
            sprite->color[4] = alpha;

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
    sprite->Render(immediateContext, 0, 0,
        Regal::Graphics::Graphics::Instance().GetScreenWidth(), 
        Regal::Graphics::Graphics::Instance().GetScreenHeight(), 0);
}

bool Fade::FadeIn(float elapsedTime)
{
    if (!hideScereen)return true;

    alpha -= elapsedTime * fadeSpeed;
    sprite->color[4] = alpha;

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
    sprite->color[4] = alpha;

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