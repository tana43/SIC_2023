#pragma once
#include <DirectXMath.h>
#include "Transform.h"

namespace Regal::Game
{
    class Camera
    {
    private:
        Camera() {}
        ~Camera() {}

    public:
        //カメラに必要な数値、この中の変数を減らすことは無い
        struct Parameters
        {
            float width;
            float height;
            float nearZ = 0.1f;
            float farZ  = 500;
            float fov   = 60;
            float aspectRatio;
        };

        static Camera& Instance()
        {
            static Camera instance;
            return instance;
        }

        void Initialize();

        void Update();
        void UpdateViewProjectionMatrix();

        DirectX::XMMATRIX CalcViewMatrix() const;
        DirectX::XMMATRIX CalcProjectionMatrix() const;

        //シーン用バッファに必要な項目だけゲッター作った。あとで変更するかも
        const DirectX::XMMATRIX GetViewProjectionMatrix() const { return ViewProjection; }
        const DirectX::XMFLOAT4 GetPosition() const 
        {
            return DirectX::XMFLOAT4(transform.GetPosition().x, transform.GetPosition().y, transform.GetPosition().z, 1.0f); 
        }

        void DrawDebug();

    private:
        DirectX::XMMATRIX ViewProjection{};

        TransformEuler transform;
        Parameters parameters;
    };
}
