#pragma once

#include <memory>
#include <d3d11.h>
#include <wrl.h>
#include <dxgi1_6.h>

//mutex:�X���b�h�ԂŎg�p���鋤�L���\�[�X��r�����䂷�邽�߂̃N���X�ł���B
//�܂蓯�����\�[�X�ɑ΂��镡���̍X�V�����ɂ���ăf�[�^�̐����������Ȃ��Ȃ邱�Ƃ�h���ł���
#include <mutex>

#include "../Resource/Shader.h"

namespace Regal::Graphics
{
	enum DEPTH_STATE { ZT_ON_ZW_ON, ZT_OFF_ZW_ON, ZT_ON_ZW_OFF, ZT_OFF_ZW_OFF };
	enum BLEND_STATE { NONE, ALPHA, ADD, MULTIPLY };
	enum RASTER_STATE { SOLID, WIREFRAME, WIREFRAME_CULL_NONE, SOLID_REVERSE, CULL_NONE };

	class Graphics
	{
	public:
		Graphics(HWND hwnd,BOOL fullscreen);
		~Graphics();

		//�V���O���g�������ǊO�����琶������K�v������̂ŃR���X�g���N�^��public
		//�ɂ���K�v������@�L���C
		static Graphics& Instance() { return *instance; }

		// �f�o�C�X�擾
		ID3D11Device* GetDevice() const { return device.Get(); }

		// �f�o�C�X�R���e�L�X�g�擾
		ID3D11DeviceContext* GetDeviceContext() const { return immediateContext.Get(); }

		// �X���b�v�`�F�[���擾
		IDXGISwapChain* GetSwapChain() const { return swapChain.Get(); }

		// �����_�[�^�[�Q�b�g�r���[�擾
		ID3D11RenderTargetView* GetRenderTargetView() const { return renderTargetView.Get(); }

		// �f�v�X�X�e���V���r���[�擾
		ID3D11DepthStencilView* GetDepthStencilView() const { return depthStencilView.Get(); }

		// �V�F�[�_�[�擾
		Regal::Resource::Shader* GetShader() const { return shader.get(); }

		// �X�N���[�����擾
		float GetScreenWidth() const { return framebufferDimensions.cx; }

		// �X�N���[�������擾
		float GetScreenHeight() const { return framebufferDimensions.cy; }

		//�~���[�e�b�N�X�擾
		std::mutex& GetMutex() { return mutex; }

		void OnSizeChanged(UINT64 width, UINT height);

		//�e��X�e�[�g�̐���
		void CreateStates();

		//�e��X�e�[�g���Z�b�g
		void SetStates(int depthStencilState, int rastarizerState, int blendState);

		void SetRSState(int rasterizerState);

		//2D�p�̃X�e�[�g���Z�b�g
		void Set2DStates();
		//3D�p�̃X�e�[�g���Z�b�g
		void Set3DStates();

		void DrawDebug();

		void FullscreenState(BOOL fullscreen);

		//�Ƃ肠���������悤�ɂ���������T���v���[�X�e�[�g�o�C���h�֐��u���Ƃ��i��ŏ����j
		void BindSamplersState()
		{
			immediateContext->PSSetSamplers(0, 1, samplerStates[0].GetAddressOf());
			immediateContext->PSSetSamplers(1, 1, samplerStates[1].GetAddressOf());
			immediateContext->PSSetSamplers(2, 1, samplerStates[2].GetAddressOf());
		}

	public:
		

		BOOL fullscreenMode{ FALSE };
		BOOL vsync{ TRUE };//��������
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

		Microsoft::WRL::ComPtr<IDXGIAdapter3> adapter;//�������̎g�p�󋵂̊m�F���ł���


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

		//�e��X�e�[�g
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
		SIZE framebufferDimensions;//�X�N���[���T�C�Y

		RECT windowedRect;
		DWORD windowedStyle;

		std::mutex mutex;
	};
}


