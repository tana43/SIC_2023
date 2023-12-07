#pragma once
#include <vector>

namespace Regal::Helper
{
    class LerpHelper
    {
    private:
        LerpHelper();
        ~LerpHelper();

    public:
        //自動更新の際に補完に必要な情報を格納する構造体
        struct LerpSubject
        {
            LerpSubject(float* subject,float before, float after, float oneWayTime = 1.0f, bool loop = false) :
                subject(subject),a(before), b(after), oneWayTime(oneWayTime), loop(loop) {}

            void Update(float elapsedTime);

            float* subject;//対象の変数

            float a;
            float b;

            float oneWayTime;
            float timer;

            bool loop;
        };

    public:
        static LerpHelper& Instance()
        {
            static LerpHelper instance;
            return instance;
        }

        void Update(float elapsedTime);

        //自動更新させる変数の登録
        LerpSubject& Register(float& variable, float beforeValue, float afterValue, float oneWayTime = 1.0f, bool loop = false);
        LerpSubject& Register(LerpSubject subject);

        //線形補間
        static float Lerp(float a,float b,float alpha,float maxAlpha = 1.0f);

        //void Slerp(T a, T b, float alpha, float maxAlpha = 1.0f);

    private:
        std::vector<LerpSubject*> subjects;
    };
}

