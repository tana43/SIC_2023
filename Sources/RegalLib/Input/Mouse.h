#pragma once

#include <Mouse.h>
#include <memory>

using MouseButton = char;

namespace Regal::Input
{
    class Mouse
    {
    private:
        Mouse();
        ~Mouse() {}

    public:
        static const MouseButton BTN_LEFT = 0;
        static const MouseButton BTN_MIDDLE = 1;
        static const MouseButton BTN_RIGHT = 2;

        static Mouse& Instance()
        {
            static Mouse instance;
            return instance;
        }

        void Update();

        //それぞれのマウスの入力状態を持った構造体を返す(例:if(GetButtonState().Space))
        static DirectX::Mouse::State& GetButtonState() { return Instance().state; }

        //キーを押した瞬間かどうかを返す
        static const bool GetButtonDown(const MouseButton button);

        //キーを離した瞬間かどうかを返す
        static const bool GetButtonUp(const MouseButton button);

        const int GetPosX() const { return state.x; }
        const int GetPosY() const { return state.y; }

        //TODO:後で前回転、後回転のどちらが正の値を返すか確認する
        //マウスホイールの回転値を返す
        const int GetScrollWheelValue() const { return state.scrollWheelValue; }

    private:
        //DirectX::Keyboardクラスを生成するためのポインタ
        std::unique_ptr<DirectX::Mouse> mouse;

        //ボタンを押した瞬間、離した瞬間の判定処理をしてくれている
        DirectX::Mouse::ButtonStateTracker tracker;

        //マウスボタンの入力状態を取ってくる
        DirectX::Mouse::State state;
    };
}

