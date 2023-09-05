#include "Keyboard.h"

namespace Regal::Input
{
    Keyboard::Keyboard()
    {
        keyboard = std::make_unique<DirectX::Keyboard>();
    }

    void Keyboard::Update()
    {
        state = keyboard->GetState();
        tracker.Update(state);
    }

    const bool Keyboard::GetKeyDown(const DirectX::Keyboard::Keys key)
    {
        return Instance().tracker.IsKeyPressed(key);
    }

    const bool Keyboard::GetKeyUp(const DirectX::Keyboard::Keys key)
    {
        return Instance().tracker.IsKeyReleased(key);
    }

    
}