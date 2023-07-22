#pragma once
#include <DirectXMath.h>
#include "MathHelper.h"
#include <string>

class Transform final
{
public:
    Transform(const DirectX::XMFLOAT3& position = DirectX::XMFLOAT3(0,0,0), 
        const DirectX::XMFLOAT3& scale = DirectX::XMFLOAT3(1, 1, 1),
        const DirectX::XMFLOAT3& rotation = DirectX::XMFLOAT3(0,0,0),
        float scaleFactor = 1.0f, 
        int coordinateSystem = 0)
        : position(position), scale(scale), rotation(rotation), scaleFactor(scaleFactor), coordinateSystem(coordinateSystem)
    {}
    ~Transform() = default;
    Transform(const Transform&) = delete;
    Transform(const Transform&&) = delete;
    Transform& operator=(const Transform&) = delete;
    Transform& operator=(const Transform&&) = delete;

    void DrawDebug();
    void Reset();

    DirectX::XMMATRIX CalcWorldMatrix();
    DirectX::XMMATRIX CalcLocalMatrix();

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
    void SetRotationY(const float rotY) { rotation.y = rotY; }
    void AddRotation(const DirectX::XMFLOAT3& rot) { rotation = rotation + rot; }
    void AddRotationY(const float rotY) { rotation.y += rotY; }

    void SetScaleFactor(const float scale) { scaleFactor = scale; }
    void AddScaleFactor(const float scale) { scaleFactor += scale; }


    //正規化済み前ベクトルを返す
    DirectX::XMFLOAT3 CalcForward() const;
    //正規化済み上ベクトルを返す
    DirectX::XMFLOAT3 CalcUp() const;
    //正規化済み右ベクトルを返す
    DirectX::XMFLOAT3 CalcRight() const;

private:
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
};

