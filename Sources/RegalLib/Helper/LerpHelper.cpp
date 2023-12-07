#include "LerpHelper.h"
#include <DirectXMath.h>

namespace Regal::Helper
{
    void LerpHelper::Update(float elapsedTime)
    {
        for (auto& subject : subjects)
        {
            subject->Update(elapsedTime);
        }
    }

    LerpHelper::LerpSubject& LerpHelper::Register(float& variable, float beforeValue, float afterValue, float oneWayTime, bool loop)
    {
        LerpSubject* subject = new LerpSubject(&variable,beforeValue,afterValue,oneWayTime,loop);
        return *subject;
    }

    LerpHelper::LerpSubject& LerpHelper::Register(LerpSubject subject)
    {
        LerpSubject* subject_ = new LerpSubject(subject.subject,subject.a,subject.b,subject.oneWayTime,subject.loop);
        return *subject_;
    }

    float LerpHelper::Lerp(float a, float b, float alpha, float maxAlpha)
    {
        if (alpha > maxAlpha)alpha = maxAlpha;
        float ab = b - a;
        ab *= alpha / maxAlpha;
        return a + ab;
    }

    void LerpHelper::LerpSubject::Update(float elapsedTime)
    {

    }
}

