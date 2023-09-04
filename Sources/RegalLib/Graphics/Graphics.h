#pragma once

#include <memory>
#include <d3d11.h>
#include <wrl.h>
#include <dxgi1_6.h>

//mutex:スレッド間で使用する共有リソースを排他制御するためのクラスである。
//つまり同じリソースに対する複数の更新処理によってデータの整合性が取れなくなることを防いでいる
#include <mutex>

#include "../Resource/Shader.h"

namespace Regal::Graphics
{
	class Graphics
	{
	public:
		Graphics(HWND hwnd,BOOL fullscreen);
		~Graphics();

		//シングルトンだけど外部から生成する必要があるのでコンストラクタはpublic
		//にする必要がある　キモイ
		static Graphics& Instance() { return *instance; }

		// デバイス取得
		ID3D11Device* GetDevice() const { return device.Get(); }

		// デバイスコンテキスト取得
		ID3D11DeviceContext* GetDeviceContext() const { return immediateContext.Get(); }

		// スワップチェーン取得
		IDXGISwapChain* GetSwapChain() const { return swapChain.Get(); }

		// レンダーターゲットビュー取得
		ID3D11RenderTargetView* GetRenderTargetView() const { return renderTargetView.Get(); }

		// デプスステンシルビュー取得
		ID3D11DepthStencilView* GetDepthStencilView() const { return depthStencilView.Get(); }

		// シェーダー取得
		Regal::Resource::Shader* GetShader() const { return shader.get(); }

		// スクリーン幅取得
		float GetScreenWidth() const { return framebufferDimensions.cx; }

		// スクリーン高さ取得
		float GetScreenHeight() const { return framebufferDimensions.cy; }

		//ミューテックス取得
		std::mutex& GetMutex() { return mutex; }

		void OnSizeChanged(UINT64 width, UINT height);

		//各種ステートの生成
		void CreateStates();

		//各種ステートをセット
		void SetStates(int depthStencilState, int rastarizeState, int blendState);

		//2D用のステートをセット
		void Set2DStates();
		//3D用のステートをセット
		void Set3DStates();

		void DrawDebug();

		void FullscreenState(BOOL fullscreen);

		//とりあえず動くようにしたいからサンプラーステートバインド関数置いとく（後で消す）
		void BindSamplersState()
		{
			immediateContext->PSSetSamplers(0, 1, samplerStates[0].GetAddressOf());
			immediateContext->PSSetSamplers(1, 1, samplerStates[1].GetAddressOf());
			immediateContext->PSSetSamplers(2, 1, samplerStates[2].GetAddressOf());
		}

	public:
		enum DEPTH_STATE { ZT_ON_ZW_ON, ZT_OFF_ZW_ON, ZT_ON_ZW_OFF, ZT_OFF_ZW_OFF };
		enum BLEND_STATE { NONE, ALPHA, ADD, MULTIPLY };
		enum RASTER_STATE { SOLID, WIREFRAME, WIREFRAME_CULL_NONE, SOLID_REVERSE, CULL_NONE };

		BOOL fullscreenMode{ FALSE };
		BOOL vsync{ FALSE };//垂直同期
		BOOL tearingSupported{ FALSE };

	private:
		void CreateSwapChain(IDXGIFactory6* dxgiFactory6);

		size_t VideoMemoryUsage()
		{
			DXGI_QUERY_VIDEO_MEMORY_INFO videoMemoryInfo;
			adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &videoMemoryInfo);
			return videoMemoryInfo.CurrentUsage / 1024 / 1024;
		}

	private:
		static Graphics* instance;

		Microsoft::WRL::ComPtr<IDXGIAdapter3> adapter;//メモリの使用状況の確認ができる


		Microsoft::WRL::ComPtr<ID3D11Device>			device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext>		immediateContext;
		Microsoft::WRL::ComPtr<IDXGISwapChain1>			swapChain;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	renderTargetView;
		Microsoft::WRL::ComPtr<ID3D11Texture2D>			depthStencilBuffer;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	depthStencilView;

		std::unique_ptr<Regal::Resource::Shader>		shader;
		//std::unique_ptr<DebugRenderer>					debugRenderer;
		//std::unique_ptr<LineRenderer>					lineRenderer;
		//std::unique_ptr<ImGuiRenderer>					imguiRenderer;

		//各種ステート
		Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerStates[3];

		

		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilStates[4];
		ID3D11DepthStencilState* setting2DDepthStencilState{ depthStencilStates[ZT_ON_ZW_ON].Get() };
		ID3D11DepthStencilState* setting3DDepthStencilState{ depthStencilStates[ZT_ON_ZW_ON].Get() };

		Microsoft::WRL::ComPtr<ID3D11BlendState> blendStates[4];
		ID3D11BlendState* setting2DBlendState{ blendStates[ALPHA].Get() };
		ID3D11BlendState* setting3DBlendState{ blendStates[ALPHA].Get() };

		Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerStates[5];
		ID3D11RasterizerState* setting2DRasterizerState{ rasterizerStates[SOLID].Get() };
		ID3D11RasterizerState* setting3DRasterizerState{ rasterizerStates[SOLID].Get() };

		CONST HWND hwnd;
		SIZE framebufferDimensions;//スクリーンサイズ

		RECT windowedRect;
		DWORD windowedStyle;

		std::mutex mutex;
	};
}


