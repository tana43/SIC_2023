#include "Mouse.h"

namespace Regal::Input
{
    Mouse::Mouse()
    {
        mouse = std::make_unique<DirectX::Mouse>();
    }
}
