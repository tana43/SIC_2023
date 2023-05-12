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

#ifdef USE_IMGUI
#include "../External/imgui/imgui.h"
#include "../External/imgui/imgui_internal.h"
#include "../External/imgui/imgui_impl_dx11.h"
#include "../External/imgui/imgui_impl_win32.h"
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern ImWchar glyphRangesJapanese[];
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
	};
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffers[8];

	void CreateSwapChain(IDXGIFactory6* dxgiFactory6);


	Framework(HWND hwnd,BOOL fullscreen);
	~Framework();

	Framework(const Framework&) = delete;
	Framework& operator=(const Framework&) = delete;
	Framework(Framework&&) noexcept = delete;
	Framework& operator=(Framework&&) noexcept = delete;

	int run()
	{
		MSG msg{};

		if (!initialize())
		{
			return 0;
		}

#ifdef USE_IMGUI
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 14.0f, nullptr, glyphRangesJapanese);
		ImGui_ImplWin32_Init(hwnd);
		ImGui_ImplDX11_Init(device.Get(), immediateContext.Get());
		ImGui::StyleColorsDark();
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
				calculate_frame_stats();
				update(tictoc.time_interval());
				render(tictoc.time_interval());
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

		return uninitialize() ? static_cast<int>(msg.wParam) : 0;
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
	bool initialize();
	void update(float elapsed_time/*Elapsed seconds from last frame*/);
	void render(float elapsed_time/*Elapsed seconds from last frame*/);
	bool uninitialize();

private:
	high_resolution_timer tictoc;
	uint32_t frames{ 0 };
	float elapsed_time{ 0.0f };
	void calculate_frame_stats()
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
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerStates[3];
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilStates[4];
	Microsoft::WRL::ComPtr<ID3D11BlendState> blendStates[4];
	std::unique_ptr<GeometricPrimitive> geometricPrimitive[8];

	float spriteColors[4] = { 1.0f,1.0f,1.0f,1.0f };

	DirectX::XMFLOAT3 cameraPos{ 0.0f, 0.0f, -10.0f};
	DirectX::XMFLOAT3 cameraAngle{ 0.0f, 0.0f, 0.0f};
	DirectX::XMFLOAT3 cameraFocus{ 0.0f, 0.0f, 0.0f};
};

