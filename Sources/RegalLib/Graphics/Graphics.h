#pragma once

#include <memory>
#include <d3d11.h>
#include <wrl.h>
#include <dxgi1_6.h>

//mutex:スレッド間で使用する共有リソースを排他制御するためのクラスである。
//つまり同じリソースに対する複数の更新処理によってデータの整合性が取れなくなることを防いでいる
#include <mutex>

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
		//Shader* GetShader() const { return shader.get(); }

		// スクリーン幅取得
		float GetScreenWidth() const { return framebufferDimensions.cx; }

		// スクリーン高さ取得
		float GetScreenHeight() const { return framebufferDimensions.cy; }

		//ミューテックス取得
		std::mutex& GetMutex() { return mutex; }

	private:
		void CreateSwapChain(IDXGIFactory6* dxgiFactory6);

		void FullscreenState(BOOL fullscreen);
		void OnSizeChanged(UINT64 width, UINT height);

	private:
		static Graphics* instance;

		Microsoft::WRL::ComPtr<IDXGIAdapter3> adapter;//メモリの使用状況の確認ができる

		Microsoft::WRL::ComPtr<ID3D11Device>			device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext>		immediateContext;
		Microsoft::WRL::ComPtr<IDXGISwapChain1>			swapChain;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	renderTargetView;
		Microsoft::WRL::ComPtr<ID3D11Texture2D>			depthStencilBuffer;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	depthStencilView;

		/*std::unique_ptr<Shader>							shader;
		std::unique_ptr<DebugRenderer>					debugRenderer;
		std::unique_ptr<LineRenderer>					lineRenderer;
		std::unique_ptr<ImGuiRenderer>					imguiRenderer;*/

		CONST HWND hwnd;
		SIZE framebufferDimensions;

		BOOL fullscreenMode{ FALSE };
		BOOL vsync{ FALSE };//垂直同期
		BOOL tearingSupported{ FALSE };

		RECT windowedRect;
		DWORD windowedStyle;

		std::mutex mutex;
	};
}


