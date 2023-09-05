#pragma once
#include <Keyboard.h>

using UseKey = unsigned int;

namespace Regal::Input
{
    //DirectXTKのKeyboardを使う
    class Keyboard
    {
    private:
        Keyboard();
        ~Keyboard() {}

    public:
        static Keyboard& Instance()
        {
            static Keyboard instance;
            return instance;
        }

        void Update();

        //それぞれのキーの入力状態を持った構造体を返す(例:if(GetKeyState().Space))
        static DirectX::Keyboard::State& GetKeyState() { return Instance().state; }

        //キーを押した瞬間かどうかを返す
        static const bool GetKeyDown(const DirectX::Keyboard::Keys key);

        //キーを離した瞬間かどうかを返す
        static const bool GetKeyUp(const DirectX::Keyboard::Keys key);

    private:
        //DirectX::Keyboardクラスを生成するためのポインタ
        std::unique_ptr<DirectX::Keyboard> keyboard;

        //ボタンを押した瞬間、離した瞬間の処理をしてくれている
        DirectX::Keyboard::KeyboardStateTracker tracker;

        //キーボードの入力状態を取ってくる
        DirectX::Keyboard::State state;
    };
}
