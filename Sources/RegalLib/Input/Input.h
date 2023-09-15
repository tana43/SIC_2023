#pragma once

#include "../Input/Mouse.h"
#include "../Input/Keyboard.h"
#include "../Input/GamePad.h"

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
        static Input& Instance()
        {
            static Input instance;
            return instance;
        }

        void Update();

        static GamePad& GetGamePad() { return Instance().gamePad; }

    private:
        GamePad gamePad;
    };
}

