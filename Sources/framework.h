#pragma once

#include <windows.h>
#include <tchar.h>
#include <sstream>


#include "misc.h"
#include "high_resolution_timer.h"

#include <d3d11.h>
#include <wrl.h>
#include <dxgi1_6.h>

#include "Sprite.h"
#include "SpriteBatch.h"
#include "GeometricPrimitive.h"
#include "StaticMesh.h"
#include "SkinnedMesh.h"
#include "GltfModel.h"

#include "FrameBuffer.h"
#include "FullscreenQuad.h"

#ifdef USE_IMGUI
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_impl_dx11.h"
#include "../imgui/imgui_impl_win32.h"
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

class Framework
{
public:
	CONST HWND hwnd;
	SIZE framebufferDimensions;

	Microsoft::WRL::ComPtr<IDXGIAdapter3> adapter;//メモリの使用状況の確認ができる
	size_t VideoMemoryUsage()
	{
		DXGI_QUERY_VIDEO_MEMORY_INFO videoMemoryInfo;
		adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &videoMemoryInfo);
		return videoMemoryInfo.CurrentUsage / 1024 / 1024;
	}

	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;

	struct  SceneConstants
	{
		DirectX::XMFLOAT4X4 viewProjection;	//ビュープロジェクション交換行列
		DirectX::XMFLOAT4 lightDirection;	//ライトの向き
		DirectX::XMFLOAT4 cameraPosition;
	};
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffers[8];

	struct ParametricConstants
	{
		float extractionThreshold{ 0.0f };
		float gaussianSigma{1.0f};
		float bloomIntensity{1.0f};
		float exposure{1.2f};
	};
	ParametricConstants parametricConstants;

	void CreateSwapChain(IDXGIFactory6* dxgiFactory6);


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

#ifdef USE_IMGUI
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui_ImplWin32_Init(hwnd);
		ImGui_ImplDX11_Init(device.Get(), immediateContext.Get());
#endif

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

#ifdef USE_IMGUI
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
#endif

#if 1
		BOOL fullscreen = 0;
		swapChain->GetFullscreenState(&fullscreen, 0);
		if (fullscreen)
		{
			swapChain->SetFullscreenState(FALSE, 0);
		}
#endif

		return Uninitialize() ? static_cast<int>(msg.wParam) : 0;
	}

	LRESULT CALLBACK handle_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
#ifdef USE_IMGUI
		if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) { return true; }
#endif
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
			if (wparam == VK_ESCAPE)
			{
				PostMessage(hwnd, WM_CLOSE, 0, 0);
			}
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
			OnSizeChanged(static_cast<UINT64>(clientRect.right - clientRect.left),clientRect.bottom - clientRect.top);

			break;
		}
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

	void SetImguiStyle();
	void DrawDebug();

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

	BOOL fullscreenMode{ FALSE };
	BOOL vsync{ TRUE };//垂直同期
	BOOL tearingSupported{ FALSE };

	RECT windowedRect;
	DWORD windowedStyle;

	void FullscreenState(BOOL fullscreen);
	void OnSizeChanged(UINT64 width, UINT height);

	std::unique_ptr<Sprite> sprites[8];
	std::unique_ptr<SpriteBatch> spritesBatches[8];
	std::unique_ptr<GeometricPrimitive> geometricPrimitive[8];
	std::unique_ptr<StaticMesh> staticMeshes[8];
	std::unique_ptr<SkinnedMesh> skinnedMeshes[8];
	std::unique_ptr<Framebuffer> frameBuffers[8];
	std::unique_ptr<GltfModel> gltfModels[8];


	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerStates[3];

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilStates[4];
	ID3D11DepthStencilState* setting2DDepthStencilState{ depthStencilStates[0].Get() };
	ID3D11DepthStencilState* setting3DDepthStencilState{ depthStencilStates[0].Get() };

	Microsoft::WRL::ComPtr<ID3D11BlendState> blendStates[4];

	float spriteColors[4] = { 1.0f,1.0f,1.0f,1.0f };

	float cameraFov{30};
	float cameraFar{100.0f};
	DirectX::XMFLOAT3 cameraPos{ 0.0f, 0.0f, -10.0f};
	DirectX::XMFLOAT3 cameraAngle{ 0.0f, 0.0f, 0.0f};
	DirectX::XMFLOAT3 cameraFocus{ 0.0f, 0.0f, 0.0f};
	DirectX::XMFLOAT3 lightAngle{ 0,0,0 };

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerStates[5];
	ID3D11RasterizerState* setting2DRasterizerState{ rasterizerStates[0].Get()};
	ID3D11RasterizerState* setting3DRasterizerState{ rasterizerStates[0].Get()};

	float boneTranslationX{ 300.0f };
	float blendAnimation{ 0.5f };

	std::unique_ptr<FullscreenQuad> bitBlockTransfer;

	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShaders[8];

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceViews[8];
};

