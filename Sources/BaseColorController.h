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

    static DirectX::XMFLOAT4 RundomBrightColor();

    DirectX::XMFLOAT4 baseColor;
};

