#include "Graphics.h"
#include "../Other/Misc.h"
#include "../../../External/imgui/imgui.h"

#include <string>

//GPU情報を取得
void AcquireHighPerformanceAdapter(IDXGIFactory6* dxgiFactory6, IDXGIAdapter3** dxgiAdapter3)
{
	HRESULT hr{ S_OK };

	Microsoft::WRL::ComPtr<IDXGIAdapter3> enumeratedAdapter;
	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != dxgiFactory6->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(enumeratedAdapter.ReleaseAndGetAddressOf())); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 adapterDesc;
		hr = enumeratedAdapter->GetDesc1(&adapterDesc);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		//VenterID 各企業毎に発行される企業固有のID
		if (adapterDesc.VendorId == 0x1002/*AMD*/ || adapterDesc.VendorId == 0x10DE/*NVIDIA*/)
		{
			OutputDebugStringW((std::wstring(adapterDesc.Description) + L" has been selected.\n").c_str());
			OutputDebugStringA(std::string("\tVendorId:" + std::to_string(adapterDesc.VendorId) + '\n').c_str());
			OutputDebugStringA(std::string("\tDeviceId:" + std::to_string(adapterDesc.DeviceId) + '\n').c_str());
			OutputDebugStringA(std::string("\tSubSysId:" + std::to_string(adapterDesc.SubSysId) + '\n').c_str());
			OutputDebugStringA(std::string("\tRevision:" + std::to_string(adapterDesc.Revision) + '\n').c_str());
			OutputDebugStringA(std::string("\tDedicatedVideoMemory:" + std::to_string(adapterDesc.DedicatedVideoMemory) + '\n').c_str());
			OutputDebugStringA(std::string("\tDedicatedSystemMemory:" + std::to_string(adapterDesc.DedicatedSystemMemory) + '\n').c_str());
			OutputDebugStringA(std::string("\tSharedSystemMemory:" + std::to_string(adapterDesc.SharedSystemMemory) + '\n').c_str());
			OutputDebugStringA(std::string("\tAdapterLuid.HighPart:" + std::to_string(adapterDesc.AdapterLuid.HighPart) + '\n').c_str());
			OutputDebugStringA(std::string("\tAdapterLuid.LowPart:" + std::to_string(adapterDesc.AdapterLuid.LowPart) + '\n').c_str());
			OutputDebugStringA(std::string("\tFlags:" + std::to_string(adapterDesc.Flags) + '\n').c_str());
			break;
		}
	}

	//GPUの情報を取得
	*dxgiAdapter3 = enumeratedAdapter.Detach();
}

namespace Regal::Graphics
{
	Graphics* Graphics::instance = nullptr;

    Graphics::Graphics(HWND hwnd, BOOL fullscreen):
		hwnd(hwnd),
		fullscreenMode(fullscreen),
		windowedStyle(static_cast<DWORD>(GetWindowLongPtrW(hwnd, GWL_STYLE)))
    {
        //インスタンス設定
        _ASSERT_EXPR(instance == nullptr, "already instantiated");
        instance = this;

		if (fullscreen)
		{
			FullscreenState(TRUE);
		}

        //画面サイズ取得
		RECT clientRect;
		GetClientRect(hwnd, &clientRect);
		framebufferDimensions.cx = clientRect.right - clientRect.left;
		framebufferDimensions.cy = clientRect.bottom - clientRect.top;

        HRESULT hr{ S_OK };

		UINT createFactoryFlags{};
#ifdef _DEBUG
		createFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#else

#endif // _DEBUG
		Microsoft::WRL::ComPtr<IDXGIFactory6> dxgiFactory6;
		hr = CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(dxgiFactory6.GetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		//アダプター最適化（搭載GPUに対応、よー分からん）
		AcquireHighPerformanceAdapter(dxgiFactory6.Get(), adapter.GetAddressOf());

		UINT createDeviceFlags{ 0 };
#ifdef _DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG

		/*D3D_FEATURE_LEVEL featureLevels[]
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0
		};
		D3D_FEATURE_LEVEL featureLevel;
		hr = D3D11CreateDevice(
			adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, 0,
			createDeviceFlags, featureLevels, _countof(featureLevels), D3D11_SDK_VERSION,
			device.ReleaseAndGetAddressOf(), &featureLevel, immediateContext.ReleaseAndGetAddressOf()
		);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		_ASSERT_EXPR(!(featureLevel < D3D_FEATURE_LEVEL_11_0), L"Direct3D Feature Level 11 unsupported.");*/
		D3D_FEATURE_LEVEL featureLevels{ D3D_FEATURE_LEVEL_11_1, };
		hr = D3D11CreateDevice(
			adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, 0,
			createDeviceFlags, &featureLevels, 1, D3D11_SDK_VERSION,
			device.ReleaseAndGetAddressOf(), NULL, immediateContext.ReleaseAndGetAddressOf()
		);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		CreateSwapChain(dxgiFactory6.Get());

		//シェーダー
		{
			shader = std::make_unique<Regal::Resource::Shader>();
		}
    }

	Graphics::~Graphics()
	{
	}

    void Graphics::CreateSwapChain(IDXGIFactory6* dxgiFactory6)
    {
		HRESULT hr{ S_OK };

		if (swapChain)
		{
			//ウィンドウサイズ変更処理
			ID3D11RenderTargetView* nullRenderTargetView{};
			immediateContext->OMSetRenderTargets(1, &nullRenderTargetView, NULL);
			renderTargetView.Reset();

			DXGI_SWAP_CHAIN_DESC swapChainDesc{};
			swapChain->GetDesc(&swapChainDesc);
			hr = swapChain->ResizeBuffers(
				swapChainDesc.BufferCount,
				framebufferDimensions.cx, framebufferDimensions.cy,
				swapChainDesc.BufferDesc.Format, swapChainDesc.Flags
			);
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

			Microsoft::WRL::ComPtr<ID3D11Texture2D> renderTargetBuffer;
			hr = swapChain->GetBuffer(0, IID_PPV_ARGS(renderTargetBuffer.GetAddressOf()));
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			D3D11_TEXTURE2D_DESC texture2dDesc;
			renderTargetBuffer->GetDesc(&texture2dDesc);

			hr = device->CreateRenderTargetView(renderTargetBuffer.Get(), NULL, renderTargetView.ReleaseAndGetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		else
		{
			BOOL allowTearing = FALSE;
			if (SUCCEEDED(hr))
			{
				hr = dxgiFactory6->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
			}
			tearingSupported = SUCCEEDED(hr) && allowTearing;

			DXGI_SWAP_CHAIN_DESC1 swapChainDesk1{};
			swapChainDesk1.Width = framebufferDimensions.cx;
			swapChainDesk1.Height = framebufferDimensions.cy;
			swapChainDesk1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDesk1.Stereo = FALSE;
			swapChainDesk1.SampleDesc.Count = 1;
			swapChainDesk1.SampleDesc.Quality = 0;
			swapChainDesk1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesk1.BufferCount = 2;
			swapChainDesk1.Scaling = DXGI_SCALING_STRETCH;
			swapChainDesk1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			swapChainDesk1.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
			swapChainDesk1.Flags = tearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
			hr = dxgiFactory6->CreateSwapChainForHwnd(device.Get(), hwnd, &swapChainDesk1, nullptr, nullptr, swapChain.ReleaseAndGetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

			//標準のフルスクリーン入力 alt+enter を無効にしている
			dxgiFactory6->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

			Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer{};
			hr = swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

			hr = device->CreateRenderTargetView(backBuffer.Get(), nullptr, renderTargetView.ReleaseAndGetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}

		Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer{};
		D3D11_TEXTURE2D_DESC texture2dDesc{};
		texture2dDesc.Width = framebufferDimensions.cx;
		texture2dDesc.Height = framebufferDimensions.cy;
		texture2dDesc.MipLevels = 1;
		texture2dDesc.ArraySize = 1;
		texture2dDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		texture2dDesc.SampleDesc.Count = 1;
		texture2dDesc.SampleDesc.Quality = 0;
		texture2dDesc.Usage = D3D11_USAGE_DEFAULT;
		texture2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		texture2dDesc.CPUAccessFlags = 0;
		texture2dDesc.MiscFlags = 0;
		hr = device->CreateTexture2D(&texture2dDesc, NULL, depthStencilBuffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));


		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		hr = device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilViewDesc, depthStencilView.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));


		D3D11_VIEWPORT viewport{};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(framebufferDimensions.cx);
		viewport.Height = static_cast<float>(framebufferDimensions.cy);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		immediateContext->RSSetViewports(1, &viewport);

		CreateStates();
    }

	void Graphics::FullscreenState(BOOL fullscreen)
	{
		fullscreenMode = fullscreen;
		if (fullscreen)
		{
			GetWindowRect(hwnd, &windowedRect);

			//ウィンドウスタイルの指定
			SetWindowLongPtrA(
				hwnd, GWL_STYLE,
				WS_OVERLAPPEDWINDOW &
				~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME)
			);//WS_OVERLAPP...~...WS_CHICKFRAME)は WS_OVERLAPPED のこと？

			RECT fullscreenWindowRect;

			{
				DEVMODE devmode{};
				devmode.dmSize = sizeof(DEVMODE);
				EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);

				fullscreenWindowRect = {
					devmode.dmPosition.x,
					devmode.dmPosition.y,
					devmode.dmPosition.x + static_cast<LONG>(devmode.dmPelsWidth),
					devmode.dmPosition.y + static_cast<LONG>(devmode.dmPelsHeight),
				};
			}
			SetWindowPos(
				hwnd,
#ifdef _DEBUG
				NULL,
#else
				HWND_TOPMOST,
#endif // _DEBUG
				fullscreenWindowRect.left,
				fullscreenWindowRect.top,
				fullscreenWindowRect.right,
				fullscreenWindowRect.bottom,
				SWP_FRAMECHANGED | SWP_NOACTIVATE
			);

			ShowWindow(hwnd, SW_MAXIMIZE);
		}
		else
		{
			SetWindowLongPtrA(hwnd, GWL_STYLE, windowedStyle);
			SetWindowPos(
				hwnd,
				HWND_NOTOPMOST,
				windowedRect.left,
				windowedRect.top,
				windowedRect.right - windowedRect.left,
				windowedRect.bottom - windowedRect.top,
				SWP_FRAMECHANGED | SWP_NOACTIVATE
			);

			ShowWindow(hwnd, SW_NORMAL);
		}
	}

	void Graphics::OnSizeChanged(UINT64 width, UINT height)
	{
		//最小化対応
		if (width == 0 || height == 0)return;

		HRESULT hr{ S_OK };
		if (width != framebufferDimensions.cx || height != framebufferDimensions.cy)
		{
			framebufferDimensions.cx = static_cast<LONG>(width);
			framebufferDimensions.cy = height;

			Microsoft::WRL::ComPtr<IDXGIFactory6> dxgiFactory6;
			hr = swapChain->GetParent(IID_PPV_ARGS(dxgiFactory6.GetAddressOf()));
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			CreateSwapChain(dxgiFactory6.Get());
		}
	}

	void Graphics::CreateStates()
	{

		HRESULT hr{ S_OK };

		D3D11_SAMPLER_DESC samplerDesc;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.MipLODBias = 0;
		samplerDesc.MaxAnisotropy = 16;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.BorderColor[0] = 0;
		samplerDesc.BorderColor[1] = 0;
		samplerDesc.BorderColor[2] = 0;
		samplerDesc.BorderColor[3] = 0;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		hr = device->CreateSamplerState(&samplerDesc, samplerStates[0].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		hr = device->CreateSamplerState(&samplerDesc, samplerStates[1].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		hr = device->CreateSamplerState(&samplerDesc, samplerStates[2].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		//深度ステンシルステートオブジェクトの生成
		{
			//深度テスト：オン,深度ライト：オン
			D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
			depthStencilDesc.DepthEnable = TRUE;
			depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilStates[0].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

			//深度テスト：オフ,深度ライト：オン
			depthStencilDesc.DepthEnable = FALSE;
			depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilStates[1].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

			//深度テスト：オン,深度ライト：オフ
			depthStencilDesc.DepthEnable = TRUE;
			depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilStates[2].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

			//深度テスト：オフ,深度ライト：オフ
			depthStencilDesc.DepthEnable = FALSE;
			depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilStates[3].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}

		//ブレンディングステートオブジェクト作成
		{
			D3D11_BLEND_DESC blend_desc{};
			blend_desc.AlphaToCoverageEnable = FALSE;
			blend_desc.IndependentBlendEnable = FALSE;
			blend_desc.RenderTarget[0].BlendEnable = FALSE;
			blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
			blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
			blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			hr = device->CreateBlendState(&blend_desc, blendStates[static_cast<size_t>(BLEND_STATE::NONE)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

			blend_desc.AlphaToCoverageEnable = FALSE;
			blend_desc.IndependentBlendEnable = FALSE;
			blend_desc.RenderTarget[0].BlendEnable = TRUE;
			blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
			blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			hr = device->CreateBlendState(&blend_desc, blendStates[static_cast<size_t>(BLEND_STATE::ALPHA)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

			blend_desc.AlphaToCoverageEnable = FALSE;
			blend_desc.IndependentBlendEnable = FALSE;
			blend_desc.RenderTarget[0].BlendEnable = TRUE;
			blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; //D3D11_BLEND_ONE D3D11_BLEND_SRC_ALPHA
			blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
			blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
			blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
			blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			hr = device->CreateBlendState(&blend_desc, blendStates[static_cast<size_t>(BLEND_STATE::ADD)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

			blend_desc.AlphaToCoverageEnable = FALSE;
			blend_desc.IndependentBlendEnable = FALSE;
			blend_desc.RenderTarget[0].BlendEnable = TRUE;
			blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO; //D3D11_BLEND_DEST_COLOR
			blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_COLOR; //D3D11_BLEND_SRC_COLOR
			blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_DEST_ALPHA;
			blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			hr = device->CreateBlendState(&blend_desc, blendStates[static_cast<size_t>(BLEND_STATE::MULTIPLY)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}

		//ラスタライザーステートオブジェクト生成
		{
			D3D11_RASTERIZER_DESC rasterizerDesc{};
			rasterizerDesc.FillMode = D3D11_FILL_SOLID;
			rasterizerDesc.CullMode = D3D11_CULL_BACK;
			rasterizerDesc.FrontCounterClockwise = FALSE;
			rasterizerDesc.DepthBias = 0;
			rasterizerDesc.DepthBiasClamp = 0;
			rasterizerDesc.SlopeScaledDepthBias = 0;
			rasterizerDesc.DepthClipEnable = TRUE;
			rasterizerDesc.ScissorEnable = FALSE;
			rasterizerDesc.MultisampleEnable = FALSE;
			rasterizerDesc.AntialiasedLineEnable = FALSE;
			hr = device->CreateRasterizerState(&rasterizerDesc, rasterizerStates[0].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

			//面カリングを逆向きにするステート
			rasterizerDesc.FrontCounterClockwise = TRUE;
			hr = device->CreateRasterizerState(&rasterizerDesc, rasterizerStates[3].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

			rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
			rasterizerDesc.CullMode = D3D11_CULL_BACK;
			rasterizerDesc.AntialiasedLineEnable = TRUE;
			hr = device->CreateRasterizerState(&rasterizerDesc, rasterizerStates[1].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

			rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
			rasterizerDesc.CullMode = D3D11_CULL_NONE;
			rasterizerDesc.AntialiasedLineEnable = TRUE;
			hr = device->CreateRasterizerState(&rasterizerDesc, rasterizerStates[2].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

			//面カリング無し
			rasterizerDesc.FillMode = D3D11_FILL_SOLID;
			rasterizerDesc.CullMode = D3D11_CULL_NONE;
			rasterizerDesc.AntialiasedLineEnable = TRUE;
			hr = device->CreateRasterizerState(&rasterizerDesc, rasterizerStates[4].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}

		//各種ステートオブジェクトセット
		{
			setting2DDepthStencilState = depthStencilStates[DEPTH_STATE::ZT_OFF_ZW_OFF].Get();
			setting3DDepthStencilState = depthStencilStates[DEPTH_STATE::ZT_ON_ZW_ON].Get();
			setting2DRasterizerState = rasterizerStates[RASTER_STATE::SOLID].Get();
			setting3DRasterizerState = rasterizerStates[RASTER_STATE::SOLID].Get();
			setting2DBlendState = blendStates[BLEND_STATE::ALPHA].Get();
			setting3DBlendState = blendStates[BLEND_STATE::ALPHA].Get();
		}
	}

	void Graphics::SetStates(int depthStencilState, int rastarizerState, int blendState)
	{
		immediateContext->RSSetState(rasterizerStates[rastarizerState].Get());
		immediateContext->OMSetDepthStencilState(depthStencilStates[depthStencilState].Get(), 1);
		immediateContext->OMSetBlendState(blendStates[blendState].Get(), nullptr, 0xFFFFFFFF);
	}

	void Graphics::Set2DStates()
	{
		immediateContext->OMSetDepthStencilState(setting2DDepthStencilState, 1);
		immediateContext->RSSetState(setting2DRasterizerState);
		immediateContext->OMSetBlendState(setting2DBlendState, nullptr, 0xFFFFFFFF);
	}

	void Graphics::Set3DStates()
	{
		immediateContext->OMSetDepthStencilState(setting3DDepthStencilState, 1);
		immediateContext->RSSetState(setting3DRasterizerState);
		immediateContext->OMSetBlendState(setting3DBlendState, nullptr, 0xFFFFFFFF);
	}

	void Graphics::DrawDebug()
	{
		if (ImGui::BeginMenu("Graphics"))
		{
			if (ImGui::BeginMenu("2D"))
			{
				/*if (ImGui::TreeNode("SpriteColor"))
				{
					ImGui::ColorPicker4("color", spriteColors, ImGuiColorEditFlags_::ImGuiColorEditFlags_AlphaBar);
					ImGui::TreePop();
				}*/

				static bool selectDFlag[4] = { false,false,false,true };
				if (ImGui::BeginMenu("DepthStencilState"))
				{
					if (ImGui::MenuItem("Z_Test ON  : Z_Write ON", "", selectDFlag[0]))
					{
						setting2DDepthStencilState = depthStencilStates[0].Get();
						selectDFlag[0] = true; selectDFlag[1] = false; selectDFlag[2] = false; selectDFlag[3] = false;
					}
					if (ImGui::MenuItem("Z_Test OFF : Z_Write ON", "", selectDFlag[1]))
					{
						setting2DDepthStencilState = depthStencilStates[1].Get();
						selectDFlag[0] = false; selectDFlag[1] = true; selectDFlag[2] = false; selectDFlag[3] = false;
					}
					if (ImGui::MenuItem("Z_Test ON  : Z_Write OFF", "", selectDFlag[2]))
					{
						setting2DDepthStencilState = depthStencilStates[2].Get();
						selectDFlag[0] = false; selectDFlag[1] = false; selectDFlag[2] = true; selectDFlag[3] = false;
					}
					if (ImGui::MenuItem("Z_Test OFF : Z_Write OFF", "", selectDFlag[3]))
					{
						setting2DDepthStencilState = depthStencilStates[3].Get();
						selectDFlag[0] = false; selectDFlag[1] = false; selectDFlag[2] = false; selectDFlag[3] = true;
					}

					ImGui::EndMenu();
				}


				static bool selectRFlag[3] = { true,false,false };
				if (ImGui::BeginMenu("RasterizerState"))
				{
					if (ImGui::MenuItem("Solid", "", selectRFlag[0]))
					{
						setting2DRasterizerState = rasterizerStates[0].Get();
						selectRFlag[0] = true; selectRFlag[1] = false; selectRFlag[2] = false;
					}
					if (ImGui::MenuItem("Wireframe", "", selectRFlag[1]))
					{
						setting2DRasterizerState = rasterizerStates[1].Get();
						selectRFlag[0] = false; selectRFlag[1] = true; selectRFlag[2] = false;
					}
					if (ImGui::MenuItem("Wireframe Culling Off", "", selectRFlag[2]))
					{
						setting2DRasterizerState = rasterizerStates[2].Get();
						selectRFlag[0] = false; selectRFlag[1] = false; selectRFlag[2] = true;
					}

					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("3D"))
			{
				static bool selectDFlag[4] = { true,false,false,false };
				if (ImGui::BeginMenu("DepthStencilState"))
				{
					if (ImGui::MenuItem("Z_Test ON  : Z_Write ON", "", selectDFlag[0]))
					{
						setting3DDepthStencilState = depthStencilStates[0].Get();
						selectDFlag[0] = true; selectDFlag[1] = false; selectDFlag[2] = false; selectDFlag[3] = false;
					}
					if (ImGui::MenuItem("Z_Test OFF : Z_Write ON", "", selectDFlag[1]))
					{
						setting3DDepthStencilState = depthStencilStates[1].Get();
						selectDFlag[0] = false; selectDFlag[1] = true; selectDFlag[2] = false; selectDFlag[3] = false;
					}
					if (ImGui::MenuItem("Z_Test ON  : Z_Write OFF", "", selectDFlag[2]))
					{
						setting3DDepthStencilState = depthStencilStates[2].Get();
						selectDFlag[0] = false; selectDFlag[1] = false; selectDFlag[2] = true; selectDFlag[3] = false;
					}
					if (ImGui::MenuItem("Z_Test OFF : Z_Write OFF", "", selectDFlag[3]))
					{
						setting3DDepthStencilState = depthStencilStates[3].Get();
						selectDFlag[0] = false; selectDFlag[1] = false; selectDFlag[2] = false; selectDFlag[3] = true;
					}

					ImGui::EndMenu();
				}


				static bool selectRFlag[4] = { true,false,false,false };
				if (ImGui::BeginMenu("RasterizerState"))
				{
					if (ImGui::MenuItem("Solid", "", selectRFlag[0]))
					{
						setting3DRasterizerState = rasterizerStates[0].Get();
						selectRFlag[0] = true; selectRFlag[1] = false; selectRFlag[2] = false; selectRFlag[3] = false;
					}
					if (ImGui::MenuItem("Wireframe", "", selectRFlag[1]))
					{
						setting3DRasterizerState = rasterizerStates[1].Get();
						selectRFlag[0] = false; selectRFlag[1] = true; selectRFlag[2] = false; selectRFlag[3] = false;
					}
					if (ImGui::MenuItem("Wireframe Culling Off", "", selectRFlag[2]))
					{
						setting3DRasterizerState = rasterizerStates[2].Get();
						selectRFlag[0] = false; selectRFlag[1] = false; selectRFlag[2] = true; selectRFlag[3] = false;
					}
					if (ImGui::MenuItem("Solid Reverse", "", selectRFlag[3]))
					{
						setting3DRasterizerState = rasterizerStates[3].Get();
						selectRFlag[0] = false; selectRFlag[1] = false; selectRFlag[2] = false; selectRFlag[3] = true;
					}

					ImGui::EndMenu();
				}

				/*ImGui::DragFloat("BoneTranslation", &boneTranslationX);

				ImGui::SliderFloat("BlendAnimation", &blendAnimation, 0.0f, 1.0f);*/

				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}
	}
}

