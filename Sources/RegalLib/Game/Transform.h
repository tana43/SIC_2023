#pragma once
#include "../Helper/MathHelper.h"
#include <string>

namespace Regal::Game
{
    class Transform
    {
    public:
        Transform(const DirectX::XMFLOAT3& position = DirectX::XMFLOAT3(0, 0, 0),
            const DirectX::XMFLOAT3& scale = DirectX::XMFLOAT3(1, 1, 1),
            const DirectX::XMFLOAT3& rotation = DirectX::XMFLOAT3(0, 0, 0),
            float scaleFactor = 1.0f,
            int coordinateSystem = 1)
            : position(position), scale(scale), rotation(rotation), scaleFactor(scaleFactor), coordinateSystem(coordinateSystem)
        {}
        ~Transform() = default;
        Transform(const Transform&) = delete;
        Transform(const Transform&&) = delete;
        Transform& operator=(const Transform&) = delete;
        Transform& operator=(const Transform&&) = delete;

        virtual void DrawDebug();
        virtual void DrawDebugPosAndRotOnly();//���W�Ɖ�]�݂̂̃f�o�b�O�\��
        void Reset();

        DirectX::XMMATRIX CalcWorldMatrix();

        DirectX::XMFLOAT3 GetPosition() const { return position; }
        DirectX::XMFLOAT3 GetScale() const { return scale; }
        DirectX::XMFLOAT3 GetRotation() const { return rotation; }
        float GetScaleFactor() const { return scaleFactor; }

        void SetPosition(const DirectX::XMFLOAT3& pos) { position = pos; }
        void SetPositionX(const float posX) { position.x = posX; }
        void SetPositionY(const float posY) { position.y = posY; }
        void SetPositionZ(const float posZ) { position.z = posZ; }
        void AddPosition(const DirectX::XMFLOAT3& pos) { position = position + pos; }

        void SetScale(const DirectX::XMFLOAT3& scl) { scale = scl; }

        void SetRotation(const DirectX::XMFLOAT3& rot) { rotation = rot; }
        void SetRotationX(const float rotX) { rotation.x = rotX; }
        void SetRotationY(const float rotY) { rotation.y = rotY; }
        void SetRotationZ(const float rotZ) { rotation.z = rotZ; }
        void AddRotation(const DirectX::XMFLOAT3& rot) { rotation = rotation + rot; }
        void AddRotationY(const float rotY) { rotation.y += rotY; }

        void SetScaleFactor(const float scale) { scaleFactor = scale; }
        void AddScaleFactor(const float scale) { scaleFactor += scale; }


        //���K���ςݑO�x�N�g����Ԃ�
        virtual DirectX::XMFLOAT3 CalcForward() const;
        //���K���ςݏ�x�N�g����Ԃ�
        virtual DirectX::XMFLOAT3 CalcUp() const;
        //���K���ς݉E�x�N�g����Ԃ�
        virtual DirectX::XMFLOAT3 CalcRight() const;

    protected:
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 scale;
        DirectX::XMFLOAT3 rotation;
        float scaleFactor;
        int coordinateSystem;

        static const int MAX_COORDINATE_SYSTEM = 4;
        const DirectX::XMFLOAT4X4 coordinateSystemTransforms[MAX_COORDINATE_SYSTEM]{
            { -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }, // 0:RHS Y-UP
            { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }, // 1:LHS Y-UP
            { -1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1 }, // 2:RHS Z-UP
            { 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 }, // 3:LHS Z-UP
        };

        const std::string coordinateSystemName[MAX_COORDINATE_SYSTEM]
        {
            "Right Hand, Y up",
            "Left Hand, Y up",
            "Right Hand, Z up",
            "Left Hand, Z up"
        };

        bool ImGuiCoordinateComboUI();

        virtual DirectX::XMMATRIX MatrixRotation() = 0;
    };

    //�I�C���[�p�ɂ���]������g����Transform
    class TransformEuler final : public Transform
    {
    public:
        TransformEuler(const DirectX::XMFLOAT3& position = DirectX::XMFLOAT3(0, 0, 0),
            const DirectX::XMFLOAT3& scale = DirectX::XMFLOAT3(1, 1, 1),
            const DirectX::XMFLOAT3& rotation = DirectX::XMFLOAT3(0, 0, 0),
            float scaleFactor = 1.0f,
            int coordinateSystem = 1) :
            Transform(position, scale, rotation, scaleFactor, coordinateSystem)
        {}
        ~TransformEuler() {}

    private:
        DirectX::XMMATRIX MatrixRotation() override;
    };

    //�N�H�[�^�j�I���ɂ���]������g����Transform
    //todo:�Ȃ񂩏o����񂩂���
    class TransformQuaternion final : public Transform
    {
    public:
        TransformQuaternion(const DirectX::XMFLOAT3& position = DirectX::XMFLOAT3(0, 0, 0),
            const DirectX::XMFLOAT3& scale = DirectX::XMFLOAT3(1, 1, 1),
            const DirectX::XMFLOAT3& rotation = DirectX::XMFLOAT3(0, 0, 0),
            float scaleFactor = 1.0f,
            int coordinateSystem = 1);
        ~TransformQuaternion() {}

        void DrawDebug() override;

        //���K���ςݑO�x�N�g����Ԃ�
        DirectX::XMFLOAT3 CalcForward()const override;
        //���K���ςݏ�x�N�g����Ԃ�
        DirectX::XMFLOAT3 CalcUp()const override;
        //���K���ς݉E�x�N�g����Ԃ�
        DirectX::XMFLOAT3 CalcRight()const override;

    private:
        DirectX::XMMATRIX MatrixRotation() override;

        DirectX::XMFLOAT4 orientation;

        float angle;
    };
}