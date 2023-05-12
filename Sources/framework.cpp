#include "framework.h"

void acquireHighPerformanceAdapter(IDXGIFactory6* dxgiFactory6, IDXGIAdapter3** dxgiAdapter3)
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

Framework::Framework(HWND hwnd,BOOL fullscreen) : hwnd(hwnd),fullscreenMode(fullscreen),windowedStyle(static_cast<DWORD>(GetWindowLongPtrW(hwnd,GWL_STYLE)))
{
	if (fullscreen)
	{
		FullscreenState(TRUE);
	}

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

	//アダプター最適化（搭載GPUに対応）
	acquireHighPerformanceAdapter(dxgiFactory6.Get(), adapter.GetAddressOf());

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
	D3D_FEATURE_LEVEL featureLevels{D3D_FEATURE_LEVEL_11_1,};
	hr = D3D11CreateDevice(
		adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, 0,
		createDeviceFlags, &featureLevels,1, D3D11_SDK_VERSION,
		device.ReleaseAndGetAddressOf(), NULL, immediateContext.ReleaseAndGetAddressOf()
	);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	CreateSwapChain(dxgiFactory6.Get());

	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
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
		//深度テスト：オン、深度ライト：オン
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilStates[0].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		//深度テスト：オフ、深度ライト：オン
		depthStencilDesc.DepthEnable = FALSE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilStates[1].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		//深度テスト：オン、深度ライト：オフ
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilStates[2].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		//深度テスト：オフ、深度ライト：オフ
		depthStencilDesc.DepthEnable = FALSE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilStates[3].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
	
		//ブレンディングステートオブジェクト作成
	{
		D3D11_BLEND_DESC blendDesc{};
		blendDesc.AlphaToCoverageEnable = FALSE;
		blendDesc.IndependentBlendEnable = FALSE;
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = device->CreateBlendState(&blendDesc, blendStates[0].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(SceneConstants);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffers[0].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	geometricPrimitive[0] = std::make_unique<GeometricPrimitive>(device.Get());

	sprites[0] = std::make_unique<Sprite>(device.Get(), L"./Resources/cyberpunk.jpg");
	sprites[1] = std::make_unique<Sprite>(device.Get(), L"./Resources/player-sprites.png");
	sprites[2] = std::make_unique<Sprite>(device.Get(), L"./Resources/fonts/font0.png");

	spritesBatches[0] = std::make_unique<SpriteBatch>(device.Get(), L"./Resources/player-sprites.png", 2048);
}

bool Framework::initialize()
{

	return true;
}

void Framework::CreateSwapChain(IDXGIFactory6* dxgiFactory6)
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
}

void Framework::update(float elapsed_time/*Elapsed seconds from last frame*/)
{
#ifdef USE_IMGUI
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif

	if (GetAsyncKeyState(VK_RETURN) & 1 && GetAsyncKeyState(VK_MENU) & 1)
	{
		FullscreenState(!fullscreenMode);
	}


#ifdef USE_IMGUI
	
	ImGui::Begin("ImGUI");
	if (ImGui::TreeNode("Camera"))
	{
		ImGui::DragFloat3("Pos", &cameraPos.x,0.1f);
		ImGui::DragFloat3("Angle", &cameraAngle.x,0.01f);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("SpriteColor"))
	{
		ImGui::ColorPicker4("Color", spriteColors, ImGuiColorEditFlags_::ImGuiColorEditFlags_AlphaBar);
		ImGui::TreePop();
	}

	geometricPrimitive[0]->DrawDebug();

	ImGui::End();

	
#endif
}
void Framework::render(float elapsed_time/*Elapsed seconds from last frame*/)
{
	HRESULT hr{ S_OK };

	FLOAT color[]{ 0.2f,0.2f,0.2f,1.0f };
	immediateContext->ClearRenderTargetView(renderTargetView.Get(), color);
	immediateContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	immediateContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());

	//深度ステートオブジェクトセット
	immediateContext->OMSetDepthStencilState(depthStencilStates[3].Get(), 1);

	//サンプラーステートオブジェクトをバインド
	immediateContext->PSSetSamplers(0, 1, samplerStates[0].GetAddressOf());
	immediateContext->PSSetSamplers(1, 1, samplerStates[1].GetAddressOf());
	immediateContext->PSSetSamplers(2, 1, samplerStates[2].GetAddressOf());

	//ブレンディングステートオブジェクトセット
	immediateContext->OMSetBlendState(blendStates[0].Get(), nullptr, 0xFFFFFFFF);

	//ビュー・プロジェクション交換行列を計算
	D3D11_VIEWPORT viewport;
	UINT numViewports{ 1 };
	immediateContext->RSGetViewports(&numViewports, &viewport);

	float aspectRaito{ viewport.Width / viewport.Height };
	DirectX::XMMATRIX P{ DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(30),aspectRaito,0.1f,100.0f) };

	DirectX::XMMATRIX Transform =DirectX::XMMatrixRotationRollPitchYaw(cameraAngle.x, cameraAngle.y, cameraAngle.z);
	DirectX::XMVECTOR Front = Transform.r[2];
	DirectX::XMFLOAT3 front;
	DirectX::XMStoreFloat3(&front, Front);
	cameraFocus = {
		cameraPos.x + front.x * 10,
		cameraPos.y + front.y * 10,
		cameraPos.z + front.z * 10
	};

	DirectX::XMVECTOR eye{ DirectX::XMVectorSet(cameraPos.x,cameraPos.y,cameraPos.z,1.0f) };
	DirectX::XMVECTOR focus{ DirectX::XMVectorSet(cameraFocus.x,cameraFocus.y,cameraFocus.z,1.0f) };
	DirectX::XMVECTOR up{ DirectX::XMVectorSet(0.0f,1.0f,0.0f,0.0f) };
	DirectX::XMMATRIX V{ DirectX::XMMatrixLookAtLH(eye,focus,up) };

	//定数バッファにセット
	SceneConstants data{};
	DirectX::XMStoreFloat4x4(&data.viewProjection, V * P);
	data.lightDirection = { 0,0,1,0 };
	immediateContext->UpdateSubresource(constantBuffers[0].Get(), 0, 0, &data, 0, 0);
	immediateContext->VSSetConstantBuffers(1, 1, constantBuffers[0].GetAddressOf());
	

//	sprites[0].get()->Render(immediateContext.Get(),
//		0.0f,0.0f,1280.0f,720.0f,
//		spriteColors[0], spriteColors[1], spriteColors[2], spriteColors[3],
//		0);
//
//	sprites[1].get()->Render(immediateContext.Get(),
//		700.0f, 200.0f, 200.0f, 200.0f,
//		spriteColors[0], spriteColors[1], spriteColors[2], spriteColors[3],
//		45,
//		0, 0, 140.0f, 240.0f);
//
//	float x{ 0 };
//	float y{ 0 };
//#if 0
//	for (size_t i = 0; i < 1092; i++)
//	{
//		sprites[1]->Render(immediateContext.Get(),
//			x, static_cast<float>(static_cast<int>(y) % 720), 64, 64,
//				1, 1, 1, 1, 0, 140*0, 240 * 0,140,240);
//		x += 32;
//		if (x > 1280 - 64)
//		{
//			x = 0;
//			y += 24;
//		}
//	}
//#else
//	spritesBatches[0]->Begin(immediateContext.Get(),nullptr,nullptr);
//	for (size_t i = 0; i < 1092; i++)
//	{
//		spritesBatches[0]->Render(immediateContext.Get(),
//			x, static_cast<float>(static_cast<int>(y) % 720), 64, 64,
//			1, 1, 1, 1, 0, 140 *0, 240 * 0,140,240);
//		x += 32;
//		if (x > 1280 - 64)
//		{
//			x = 0;
//			y += 24;
//		}
//	}
//	spritesBatches[0]->End(immediateContext.Get());
//#endif

	/*prites[2]->Textout(immediateContext.Get(), "FULL SCREEN : alt + enter",0,0,30,30,1,1,1,1);*/

	geometricPrimitive[0]->Render(immediateContext.Get());

#ifdef USE_IMGUI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif

	UINT syncInterval{ vsync ? 1U : 0U };
	UINT flags = (tearingSupported && !fullscreenMode && !vsync) ? DXGI_PRESENT_ALLOW_TEARING : 0;
	hr = swapChain->Present(syncInterval, flags);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

bool Framework::uninitialize()
{
	return true;
}

void Framework::FullscreenState(BOOL fullscreen)
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

void Framework::OnSizeChanged(UINT64 width, UINT height)
{
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

Framework::~Framework()
{

}