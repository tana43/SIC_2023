#pragma once

#include "../../../External/DirectXTK-main/Inc/Mouse.h"
#include <memory>

namespace Regal::Input
{
    class Mouse
    {
    private:
        Mouse();
        ~Mouse() {}

    public:
        static Mouse& Instance()
        {
            static Mouse instance;
            return instance;
        }

    private:
        std::unique_ptr<DirectX::Mouse> mouse;

    };
}

