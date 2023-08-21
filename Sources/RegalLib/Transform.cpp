#include "Transform.h"
#include "../../External/imgui/imgui.h"
#include <vector>


void Transform::DrawDebug()
{
    ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
    if (ImGui::TreeNode("Transform"))
    {
        ImGui::DragFloat3("Position", &position.x, 0.1f, -FLT_MAX, FLT_MAX);
        ImGui::DragFloat3("Scale", &scale.x, 0.01f, -FLT_MAX, FLT_MAX);
        ImGui::DragFloat3("Rotation", &rotation.x, 0.01f, -FLT_MAX, FLT_MAX);
        ImGui::DragFloat("ScaleFactor", &scaleFactor, 0.01f, 0.01f, 100.0f);

        /*ImGui::SetCursorPos(ImVec2(0, 0));
        if (ImGui::Button("Reset Position", ImVec2(100, 100)))
        {
            Reset();
        }
        ImGui::SetItemAllowOverlap();
        ImGui::SetCursorPos(ImVec2(0, 0));
        if (ImGui::Button("Reset Scale", ImVec2(50, 50)))
        {
            Reset();
        }*/

        ImGuiCoordinateComboUI();

        ImGui::TreePop();
    }
}

void Transform::Reset()
{
    position = DirectX::XMFLOAT3(0,0,0);
    scale  = DirectX::XMFLOAT3(1,1,1);
    rotation = DirectX::XMFLOAT3(0,0,0);
    scaleFactor = 1.0f;
}

DirectX::XMMATRIX Transform::CalcWorldMatrix()
{
    //スケール
    const DirectX::XMMATRIX S{DirectX::XMMatrixScaling(
        scale.x * scaleFactor, scale.y * scaleFactor, scale.z * scaleFactor)};

    //回転
    //継承先のクラスで回転行列作成の関数を定義する(オイラー、クォータニオンそれぞれに対応するため)
    const DirectX::XMMATRIX R{MatrixRotation()};

    //座標
    const DirectX::XMMATRIX T{DirectX::XMMatrixTranslation(position.x, position.y, position.z)};

    //座標系に合わせた変換
    const DirectX::XMMATRIX C{DirectX::XMLoadFloat4x4(&coordinateSystemTransforms[coordinateSystem])};

    return S * R * T * C;
}

DirectX::XMFLOAT3 Transform::CalcForward() const
{
    DirectX::XMMATRIX rotationMatrix{DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z)};
    DirectX::XMFLOAT3 ret{};
    DirectX::XMStoreFloat3(&ret, DirectX::XMVector3Normalize(rotationMatrix.r[2]));
    return ret;
}

DirectX::XMFLOAT3 Transform::CalcUp() const
{
    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
    DirectX::XMFLOAT3 ret{};
    DirectX::XMStoreFloat3(&ret, DirectX::XMVector3Normalize(rotationMatrix.r[1]));
    return ret;
}

DirectX::XMFLOAT3 Transform::CalcRight() const
{
    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
    DirectX::XMFLOAT3 ret{};
    DirectX::XMStoreFloat3(&ret, DirectX::XMVector3Normalize(rotationMatrix.r[0]));
    return ret;
}

bool Transform::ImGuiCoordinateComboUI()
{
    bool changed = false;

    if (ImGui::BeginCombo("Coordinate System", coordinateSystemName[coordinateSystem].c_str())) {
        for (int n = 0; n < MAX_COORDINATE_SYSTEM; n++) {
            bool isSelected = (coordinateSystem == n);
            if (ImGui::Selectable(coordinateSystemName[n].c_str(), isSelected)) {
                coordinateSystem = n;
                changed = true;
            }
            if (isSelected) {
                //選択時一度だけ呼ばれる処理はここに書く

                // Set the initial focus when opening the combo (scrolling + for
                // keyboard navigation support in the upcoming navigation branch)
               // ImGui::SetItemDefaultFertexocus();
            }
        }
        ImGui::EndCombo();
    }

    return changed;
}

DirectX::XMMATRIX TransformEuler::MatrixRotation()
{
    return DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
}

TransformQuaternion::TransformQuaternion(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& scale, const DirectX::XMFLOAT3& rotation, float scaleFactor, int coordinateSystem)
    : Transform(position, scale, rotation, scaleFactor, coordinateSystem)
{
    DirectX::XMFLOAT3 n(0, 1, 0);
    angle = DirectX::XMConvertToRadians(0);

    orientation = {
        n.x * sinf(angle / 2.0f),
        n.y * sinf(angle / 2.0f),
        n.z * sinf(angle / 2.0f),
        cosf(angle / 2.0f)
    };
}

void TransformQuaternion::DrawDebug()
{
    ImGui::SetNextItemOpen(true,ImGuiCond_FirstUseEver);
    if (ImGui::TreeNode("Transform"))
    {
        ImGui::DragFloat3("Position", &position.x, 0.1f, -FLT_MAX, FLT_MAX);
        ImGui::DragFloat3("Scale", &scale.x, 0.01f, -FLT_MAX, FLT_MAX);
        ImGui::DragFloat3("Rotation", &rotation.x, 0.01f, -FLT_MAX, FLT_MAX);
        ImGui::DragFloat("ScaleFactor", &scaleFactor, 0.01f, 0.01f, 100.0f);
        ImGui::SliderAngle("Angle", &angle);
        /*ImGui::SetCursorPos(ImVec2(0, 0));
        if (ImGui::Button("Reset Position", ImVec2(100, 100)))
        {
            Reset();
        }
        ImGui::SetItemAllowOverlap();
        ImGui::SetCursorPos(ImVec2(0, 0));
        if (ImGui::Button("Reset Scale", ImVec2(50, 50)))
        {
            Reset();
        }*/

        ImGuiCoordinateComboUI();

        ImGui::TreePop();
    }
}

DirectX::XMFLOAT3 TransformQuaternion::CalcForward() const
{
    DirectX::XMMATRIX rotationMatrix{DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&orientation))};
    DirectX::XMFLOAT3 ret{};
    DirectX::XMStoreFloat3(&ret, DirectX::XMVector3Normalize(rotationMatrix.r[2]));
    return ret;
}

DirectX::XMFLOAT3 TransformQuaternion::CalcUp() const
{
    DirectX::XMMATRIX rotationMatrix{DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&orientation))};
    DirectX::XMFLOAT3 ret{};
    DirectX::XMStoreFloat3(&ret, DirectX::XMVector3Normalize(rotationMatrix.r[1]));
    return ret;
}

DirectX::XMFLOAT3 TransformQuaternion::CalcRight() const
{
    DirectX::XMMATRIX rotationMatrix{DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&orientation))};
    DirectX::XMFLOAT3 ret{};
    DirectX::XMStoreFloat3(&ret, DirectX::XMVector3Normalize(rotationMatrix.r[0]));
    return ret;
}

inline float normalizeAngle(float& radian)
{
    //角度の正規化（-180～180度／-π～πラジアン）をする
    while (radian > DirectX::XM_PI) radian -= DirectX::XM_2PI;
    while (radian < -DirectX::XM_PI) radian += DirectX::XM_2PI;

    return radian;
}

DirectX::XMMATRIX TransformQuaternion::MatrixRotation()
{
    DirectX::XMVECTOR orientationVec{DirectX::XMLoadFloat4(&orientation)};
    
    float roll{ normalizeAngle(rotation.z)};
    float pitch{ normalizeAngle(rotation.x)};
    float yaw{ normalizeAngle(rotation.y)};

    /*DirectX::XMVECTOR rot{DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll)};
    orientationVec = DirectX::XMQuaternionMultiply(orientationVec, rot);*/

    DirectX::XMVECTOR forward, right, up;
    DirectX::XMMATRIX m = DirectX::XMMatrixRotationQuaternion(orientationVec);
    DirectX::XMFLOAT4X4 m4x4 = {};
    DirectX::XMStoreFloat4x4(&m4x4, m);

    right = { m4x4._11,m4x4._12,m4x4._13 };
    up = { m4x4._21,m4x4._22,m4x4._23 };
    forward = { m4x4._31,m4x4._32,m4x4._33 };

    DirectX::XMVECTOR q;
    q = DirectX::XMQuaternionRotationAxis(up, yaw);
    orientationVec = DirectX::XMQuaternionMultiply(orientationVec, q);

    q = DirectX::XMQuaternionRotationAxis(right, pitch);
    orientationVec = DirectX::XMQuaternionMultiply(orientationVec, q);

    q = DirectX::XMQuaternionRotationAxis(forward, roll);
    orientationVec = DirectX::XMQuaternionMultiply(orientationVec, q);


    DirectX::XMMATRIX R{DirectX::XMMatrixRotationQuaternion(orientationVec)};
    return R;
}
