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

    //Z�F�������_���Ŏ擾
    static DirectX::XMFLOAT4 GetRundomBrightColor();

    DirectX::XMFLOAT4 baseColor;

    //�w�i�p�p�[�e�B�N��
    std::unique_ptr<Regal::Graphics::Particles> BGParticles;

    //�p�[�e�B�N���̐F�ύX���o�t���O
    bool isBGColorChange;
    float changeTimer;
    float changeMaxTime = 1.5f;

    DirectX::XMFLOAT4 particleColor;
};

