#include "Input.h"

namespace Regal::Input
{
    Input::Input()
    {
    }

    void Input::Update()
    {
        Keyboard::Instance().Update();
        Mouse::Instance().Update();
        gamePad.Update();
    }
}
