#pragma once

#include <windows.h>
#include <tchar.h>
#include <sstream>

#include "HighResolutionTimer.h"

#include <dxgi1_6.h>
#include <Keyboard.h>
#include <Mouse.h>

#include "RegalLib/Regal.h"

#ifdef USE_IMGUI
#include "../External/imgui/ImGuiCtrl.h"
#endif

class Framework
{
public:
	CONST HWND hwnd;

	//struct  SceneConstants
	//{
	//	DirectX::XMFLOAT4X4 viewProjection;	//ビュープロジェクション交換行列
	//	DirectX::XMFLOAT4 lightDirection;	//ライトの向き
	//	DirectX::XMFLOAT4 cameraPosition;
	//	//DirectX::XMFLOAT4X4 inverseViewProjection;//ビュープロジェクション逆行列
	//};
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffers[8];

	struct ParametricConstants
	{
		float extractionThreshold{ 0.0f };
		float gaussianSigma{1.0f};
		float bloomIntensity{1.0f};
		float exposure{1.2f};
	};
	ParametricConstants parametricConstants;


	Framework(HWND hwnd,BOOL fullscreen);
	~Framework();

	Framework(const Framework&) = delete;
	Framework& operator=(const Framework&) = delete;
	Framework(Framework&&) noexcept = delete;
	Framework& operator=(Framework&&) noexcept = delete;

	int Run()
	{
		MSG msg{};

		if (!Initialize())
		{
			return 0;
		}

		// ImGui初期化(DirectX11の初期化の下に置くこと)
		IMGUI_CTRL_INITIALIZE(hwnd, graphics.GetDevice(), graphics.GetDeviceContext());

		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				tictoc.tick();
				CalculateFrameStats();
				Update(tictoc.time_interval());
				Render(tictoc.time_interval());
			}
		}

		// ImGui終了化
		IMGUI_CTRL_UNINITIALIZE();

#if 1
		BOOL fullscreen = 0;
		Regal::Graphics::Graphics::Instance().GetSwapChain()->GetFullscreenState(&fullscreen, 0);
		if (fullscreen)
		{
			Regal::Graphics::Graphics::Instance().GetSwapChain()->SetFullscreenState(FALSE, 0);
		}
#endif

		return Uninitialize() ? static_cast<int>(msg.wParam) : 0;
	}

	LRESULT CALLBACK handle_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		// ImGui(先頭に置く)
		IMGUI_CTRL_WND_PRC_HANDLER(hwnd, msg, wparam, lparam);
		
		switch (msg)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT ps{};
			BeginPaint(hwnd, &ps);

			EndPaint(hwnd, &ps);
		}
		break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_CREATE:
			break;
		case WM_KEYDOWN:

		case WM_KEYUP:
		case WM_SYSKEYUP:
			if (wparam == VK_ESCAPE)
			{
				PostMessage(hwnd, WM_CLOSE, 0, 0);
			}

			DirectX::Keyboard::ProcessMessage(msg, wparam, lparam);
			break;
		case WM_ENTERSIZEMOVE:
			tictoc.stop();
			break;
		case WM_EXITSIZEMOVE:
			tictoc.start();
			break;
		case WM_SIZE:
		{
			RECT clientRect{};
			GetClientRect(hwnd, &clientRect);
			Regal::Graphics::Graphics::Instance().OnSizeChanged(static_cast<UINT64>(clientRect.right - clientRect.left),clientRect.bottom - clientRect.top);

			break;
		}
		case WM_ACTIVATE:
		case WM_ACTIVATEAPP:
			DirectX::Keyboard::ProcessMessage(msg, wparam, lparam);
		case WM_INPUT:
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		case WM_MOUSEHOVER:
			DirectX::Mouse::ProcessMessage(msg, wparam, lparam);
			break;

		case WM_SYSKEYDOWN:
			if (wparam == VK_RETURN && (lparam & 0x60000000) == 0x20000000)
			{
				// This is where you'd implement the classic ALT+ENTER hotkey for fullscreen toggle
			}
			DirectX::Keyboard::ProcessMessage(msg, wparam, lparam);
			break;
		default:
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}
		return 0;
	}

private:
	bool Initialize();
	void Update(float elapsed_time/*Elapsed seconds from last frame*/);
	void Render(float elapsed_time/*Elapsed seconds from last frame*/);
	bool Uninitialize();

	void DrawDebug();

private:
	Regal::Graphics::Graphics graphics;

private:

	float color[4] = { 0.1f,0.1f,0.1f,1.0f };

	high_resolution_timer tictoc;
	uint32_t frames{ 0 };
	float elapsed_time{ 0.0f };
	void CalculateFrameStats()
	{
		if (++frames, (tictoc.time_stamp() - elapsed_time) >= 1.0f)
		{
			float fps = static_cast<float>(frames);
			std::wostringstream outs;
			outs.precision(6);
			outs << L" : FPS : " << fps << L" / " << L"Frame Time : " << 1000.0f / fps << L" (ms)";
			SetWindowTextW(hwnd, outs.str().c_str());

			frames = 0;
			elapsed_time += 1.0f;
		}
	}

	/*std::unique_ptr<Regal::Resource::Sprite> sprites[8];
	std::unique_ptr<Regal::Resource::SpriteBatch> spritesBatches[8];
	std::unique_ptr<Regal::Resource::GeometricPrimitive> geometricPrimitive[8];
	std::unique_ptr<Regal::Resource::StaticMesh> staticMeshes[8];
	std::unique_ptr<Regal::Resource::SkinnedMesh> skinnedMeshes[8];
	std::unique_ptr<Regal::Resource::GltfModel> gltfModels[8];
	std::unique_ptr<Regal::Graphics::Framebuffer> framebuffers[8];
	std::unique_ptr<Regal::Graphics::SkyBox> skybox;
	std::unique_ptr<Regal::Resource::Sprite> skyboxSprite;
	std::unique_ptr<Regal::Graphics::Particles> particles;*/


	float spriteColors[4] = { 1.0f,1.0f,1.0f,1.0f };

	//float cameraFov{30};
	//float cameraFar{100.0f};
	//DirectX::XMFLOAT3 cameraPos{ 0.0f, 0.0f, -10.0f};
	//DirectX::XMFLOAT3 cameraAngle{ 0.0f, 0.0f, 0.0f};
	//DirectX::XMFLOAT3 cameraFocus{ 0.0f, 0.0f, 0.0f};
	//DirectX::XMFLOAT3 lightAngle{ 0,0,0 };

	//float boneTranslationX{ 300.0f };
	//float blendAnimation{ 0.5f };

	std::unique_ptr<Regal::Graphics::FullscreenQuad> bitBlockTransfer;

	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShaders[8];

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceViews[8];

	std::unique_ptr<Regal::Graphics::Bloom> bloomer;
};

