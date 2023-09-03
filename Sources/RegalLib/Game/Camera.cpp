#include "Camera.h"
#include <d3d11.h>
#include "../Graphics/Graphics.h"

#include "../../../External/imgui/imgui.h"

namespace Regal::Game
{
    void Camera::Initialize()
    {
		transform.SetPosition(DirectX::XMFLOAT3(0, 0, -10));
		//transform.SetRotationY(DirectX::XMConvertToRadians(180));
    }

    void Camera::Update()
    {
		
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

		ViewProjection = P * V;
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
		
		Focus = DirectX::XMLoadFloat4(&focus);
		/*-----------------------------------------------------------------------*/

		//Eye.m128_f32[3] = Focus.m128_f32[3] = 1.0f;

		//DirectX::XMVECTOR Up{DirectX::XMVectorSet(0,1,0,0.0f)};

		DirectX::XMFLOAT3 up = DirectX::XMFLOAT3(0,1,0);
		DirectX::XMVECTOR Up{DirectX::XMLoadFloat3(&up)};

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
		if (ImGui::TreeNode("Camera"))
		{
			ImGui::SliderFloat("Fov", &parameters.fov,1.0f,110.0f);
			ImGui::DragFloat("NearZ", &parameters.nearZ, 1.0f,0.1f);
			ImGui::DragFloat("FarZ", &parameters.farZ, 1.0f,0.1f);

			transform.DrawDebugPosAndRotOnly();

			ImGui::InputFloat("Width",&parameters.width);
			ImGui::InputFloat("Height",&parameters.height);
			ImGui::InputFloat("AspectRatio",&parameters.aspectRatio);

			ImGui::TreePop();
		}
	}
}
