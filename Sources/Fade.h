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

    //�����Ō����t�F�[�h�C���̓Q�[����ʂ������Ă�������
    bool FadeIn(float elapsedTime);
    bool FadeOut(float elapsedTime);

    //���ĂԂ����ōŌ�܂Ńt�F�[�h����
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

