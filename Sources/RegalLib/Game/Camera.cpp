#include "Camera.h"
#include <d3d11.h>
#include "../Graphics/Graphics.h"
#include "../../Easing.h"

#include "../../../External/imgui/imgui.h"

namespace Regal::Game
{
    void Camera::Initialize()
    {
		transform.SetPosition(DirectX::XMFLOAT3(0, 0, -10));
		//transform.SetRotationY(DirectX::XMConvertToRadians(180));
    }

    void Camera::Update(float elapsedTime)
    {
		ScreenVibrationUpdate(elapsedTime);
    }

	void Camera::UpdateViewProjectionMatrix()
	{
		D3D11_VIEWPORT viewport{};
		UINT numViewports{ 1 };
		auto immediateContext{
			Regal::Graphics::Graphics::Instance().GetDeviceContext()
		};
		immediateContext->RSGetViewports(&numViewports, &viewport);

		parameters.width = viewport.Width;
		parameters.height = viewport.Height;
		parameters.aspectRatio = viewport.Width / viewport.Height;

		const DirectX::XMMATRIX P{CalcProjectionMatrix()};
		const DirectX::XMMATRIX V{CalcViewMatrix()};

		ViewProjection = V * P;
	}

    DirectX::XMMATRIX Camera::CalcViewMatrix() const
    {
		DirectX::XMFLOAT3 forward = transform.CalcForward();

		/*-----------------------------視点設定-------------------------------------*/
		DirectX::XMVECTOR Eye;

		DirectX::XMFLOAT4 eye
		{
			transform.GetPosition().x,
			transform.GetPosition().y,
			transform.GetPosition().z,
			1.0f
		};
		
		Eye = DirectX::XMLoadFloat4(&eye);
		/*-----------------------------------------------------------------------*/

		/*---------------------------注視点設定-----------------------------------*/
		DirectX::XMVECTOR Focus;

		DirectX::XMFLOAT4 focus
		{
			eye.x + forward.x * 10.0f,
			eye.y + forward.y * 10.0f,
			eye.z + forward.z * 10.0f,
			1.0f
		};

		//画面振動
		focus.x += screenVibrationOffset.x;
		focus.y += screenVibrationOffset.y;
		focus.z += screenVibrationOffset.z;
		
		Focus = DirectX::XMLoadFloat4(&focus);
		/*-----------------------------------------------------------------------*/

		//Eye.m128_f32[3] = Focus.m128_f32[3] = 1.0f;

		DirectX::XMVECTOR Up{DirectX::XMVectorSet(0.0f,1.0f,0.0f,0.0f)};

		//DirectX::XMFLOAT3 up = DirectX::XMFLOAT3(0,1,0);
		//DirectX::XMVECTOR Up{DirectX::XMLoadFloat3(&up)};

		return DirectX::XMMatrixLookAtLH(Eye, Focus, Up);
    }

    DirectX::XMMATRIX Camera::CalcProjectionMatrix() const
    {
        return DirectX::XMMatrixPerspectiveFovLH(
			DirectX::XMConvertToRadians(parameters.fov), 
			parameters.aspectRatio, 
			parameters.nearZ, 
			parameters.farZ
		);
    }

	//シーン用のImguiメニューに出す
	void Camera::DrawDebug()
	{
		if (ImGui::BeginMenu("Camera"))
		{
			ImGui::SliderFloat("Fov", &parameters.fov,1.0f,110.0f);
			ImGui::DragFloat("NearZ", &parameters.nearZ, 1.0f,0.1f);
			ImGui::DragFloat("FarZ", &parameters.farZ, 1.0f,0.1f);

			transform.DrawDebugPosAndRotOnly();

			ImGui::InputFloat("Width",&parameters.width);
			ImGui::InputFloat("Height",&parameters.height);
			ImGui::InputFloat("AspectRatio",&parameters.aspectRatio);

			static float vibVolume;
			static float vibTime;
			if (ImGui::TreeNode("Vibration"))
			{
				ImGui::SliderFloat("VibrationVolume", &vibVolume, 0.0f, 1.0f);
				ImGui::SliderFloat("VibrationTime", &vibTime, 0.0f, 5.0f);
				if (ImGui::Button("Vibrate"))
				{
					ScreenVibrate(vibVolume, vibTime);
				}
				ImGui::TreePop();
			}

			ImGui::EndMenu();
		}
	}

	void Camera::ScreenVibrate(float volume, float effectTime)
	{
		vibrationVolume = volume;
		vibrationTimer = effectTime;
		vibrationTime = effectTime;
	}

	void Camera::ScreenVibrationUpdate(float elapsedTime)
	{
		screenVibrationOffset = {};
		if (vibrationTimer <= 0)return;

		//振動方向の指定(乱数)
		DirectX::XMFLOAT3 vibVec;
		auto right = GetTransform()->CalcRight();
		auto up = GetTransform()->CalcUp();

		right = right * (rand() % 100 - 50.0f);
		up = up *(rand() % 100 - 50.0f);

		vibVec = {
			right.x + up.x,
			right.y + up.y,
			0.0f
		};
		vibVec = Normalize(vibVec);

		//イージングを使い経過時間で振動量を調整する
		float vibrationVolume = Easing::InSine(vibrationTimer, vibrationTime, this->vibrationVolume, 0.0f);

		//振動値を入れる
		screenVibrationOffset = vibVec * vibrationVolume;

		vibrationTimer -= elapsedTime;
	}

	void Camera::VerticalCameraShake(float volume, float effectTime)
	{
		
	}
}
