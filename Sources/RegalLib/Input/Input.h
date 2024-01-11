#pragma once

#include "../Input/Mouse.h"
#include "../Input/Keyboard.h"
#include "../Input/GamePad.h"

using Device = unsigned int;

namespace Regal::Input
{
    //このクラスから各種入力のアップデートをする
    //入力判定を各アクションごとに決めたいときもここで関数定義する
    class Input
    {
    private:
        Input();
        ~Input() {}

    public:
        static const Device KEYBOARD = 0;
        static const Device GAMEPAD = 1;

    public:
        static Input& Instance()
        {
            static Input instance;
            return instance;
        }

        void Update();

        static GamePad& GetGamePad() { return Instance().gamePad; }

        const Device& GetUseDevice() const { return useDevice; }

    private:
        GamePad gamePad;

        Device useDevice;
    };
}

