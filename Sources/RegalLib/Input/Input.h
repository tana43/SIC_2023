#pragma once
#include <GamePad.h>
#include <Keyboard.h>
#include <Mouse.h>

namespace Regal::Input
{
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

        //最低限使いそうなキーのみ定義
        enum UseKey
        {
            ENTER,
            W,A,S,D,
            Q,E,R,F,
            SPACE,
            I,J,K,L,
            L_SHIFT,
            L_CTRL,
        };

        void Update();

    private:
        std::unique_ptr<DirectX::Keyboard> keyboard;

        
    };
}

