#include "Mouse.h"

namespace Regal::Input
{
    Mouse::Mouse()
    {
        mouse = std::make_unique<DirectX::Mouse>();
    }

    void Mouse::Update()
    {
        state = mouse->GetState();
        tracker.Update(state);
    }

    const bool Mouse::GetButtonDown(const MouseButton button)
    {
        DirectX::Mouse::ButtonStateTracker::ButtonState thisButton;
        auto tracker{ Instance().tracker };
        switch (button)
        {
        case BTN_LEFT:  thisButton = tracker.leftButton;   break;
        case BTN_MIDDLE:thisButton = tracker.middleButton; break;
        case BTN_RIGHT: thisButton = tracker.rightButton;  break;
        default: return false; break;
        }
        return thisButton == DirectX::Mouse::ButtonStateTracker::PRESSED;
    }

    const bool Mouse::GetButtonUp(const MouseButton button)
    {
        DirectX::Mouse::ButtonStateTracker::ButtonState thisButton;
        auto tracker{ Instance().tracker };
        switch (button)
        {
        case BTN_LEFT:  thisButton = tracker.leftButton;   break;
        case BTN_MIDDLE:thisButton = tracker.middleButton; break;
        case BTN_RIGHT: thisButton = tracker.rightButton;  break;
        default: return false; break;
        }
        return thisButton == DirectX::Mouse::ButtonStateTracker::RELEASED;
    }
}
