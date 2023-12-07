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
        //�����X�V�̍ۂɕ⊮�ɕK�v�ȏ����i�[����\����
        struct LerpSubject
        {
            LerpSubject(float* subject,float before, float after, float oneWayTime = 1.0f, bool loop = false) :
                subject(subject),a(before), b(after), oneWayTime(oneWayTime), loop(loop) {}

            void Update(float elapsedTime);

            float* subject;//�Ώۂ̕ϐ�

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

        //�����X�V������ϐ��̓o�^
        LerpSubject& Register(float& variable, float beforeValue, float afterValue, float oneWayTime = 1.0f, bool loop = false);
        LerpSubject& Register(LerpSubject subject);

        //���`���
        static float Lerp(float a,float b,float alpha,float maxAlpha = 1.0f);

        //void Slerp(T a, T b, float alpha, float maxAlpha = 1.0f);

    private:
        std::vector<LerpSubject*> subjects;
    };
}

