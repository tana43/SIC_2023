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
        //�J�����ɕK�v�Ȑ��l�A���̒��̕ϐ������炷���Ƃ͖���
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

        TransformEuler* GetTransform() { return &transform; }

        //�V�[���p�o�b�t�@�ɕK�v�ȍ��ڂ����Q�b�^�[������B���ƂŕύX���邩��
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
