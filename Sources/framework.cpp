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
		D3D11_BLEND_DESC blendDesc{};
		blendDesc.AlphaToCoverageEnable = FALSE;
		blendDesc.IndependentBlendEnable                = FALSE;
		blendDesc.RenderTarget[0].BlendEnable           = TRUE;
		blendDesc.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
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
	
	//sprites[0] = std::make_unique<Sprite>(device.Get(), L"./Resources/cyberpunk.jpg");
	//sprites[1] = std::make_unique<Sprite>(device.Get(), L"./Resources/player-sprites.png");
	//sprites[2] = std::make_unique<Sprite>(device.Get(), L"./Resources/fonts/font0.png");

	spritesBatches[0] = std::make_unique<SpriteBatch>(device.Get(), L"./Resources/player-sprites.png", 2048);

	geometricPrimitive[0] = std::make_unique<GeometricPrimitive>(device.Get());
	geometricPrimitive[1] = std::make_unique<GeometricPrimitive>(device.Get(),
		GeometricPrimitive::MeshType::Cylinder, DirectX::XMFLOAT3(-1.5f, 0, 0), DirectX::XMFLOAT4(0.1f, 0.8f, 0.2f, 1.0f));

	staticMeshes[0] = std::make_unique<StaticMesh>(device.Get(),L"./Resources/F-14A_Tomcat/F-14A_Tomcat.obj", true, DirectX::XMFLOAT3(1.5f, 0, 0));
	//staticMeshes[0] = std::make_unique<StaticMesh>(device.Get(),L"./Resources/Cube.obj", true, DirectX::XMFLOAT3(1.5f, 0, 0));
	staticMeshes[1] = std::make_unique<StaticMesh>(device.Get(),L"./Resources/Rock/Rock.obj", true);

	skinnedMeshes[0] = std::make_unique<SkinnedMesh>(device.Get(), "./resources/cube.000.fbx");

	//各種ステートオブジェクトセット
	{
		setting2DDepthStencilState = depthStencilStates[3].Get();
		setting3DDepthStencilState = depthStencilStates[0].Get();
		setting2DRasterizerState = rasterizerStates[0].Get();
		setting3DRasterizerState = rasterizerStates[0].Get();
	}
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

	//ラスタライザーステートオブジェクト生成
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

}

void Framework::update(float elapsed_time/*Elapsed seconds from last frame*/)
{
#ifdef USE_IMGUI
	//static変数は一度しか初期化されないので、ラムダの中身を一度だけ呼ぶようにしている
	static bool CallOnce = [&]() { SetImguiStyle(); return true; }();
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif

	if (GetAsyncKeyState(VK_RETURN) & 1 && GetAsyncKeyState(VK_MENU) & 1)
	{
		FullscreenState(!fullscreenMode);
	}

	DrawDebug();

	//ImGui::ShowDemoWindow();
	
}
void Framework::render(float elapsed_time/*Elapsed seconds from last frame*/)
{
	HRESULT hr{ S_OK };

	FLOAT color[]{ 0.2f,0.2f,0.2f,1.0f };
	immediateContext->ClearRenderTargetView(renderTargetView.Get(), color);
	immediateContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	immediateContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());

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
	DirectX::XMMATRIX lightDirection{ DirectX::XMMatrixRotationRollPitchYaw(lightAngle.x,lightAngle.y,lightAngle.z) };
	DirectX::XMStoreFloat3(&front,lightDirection.r[2]);
	data.lightDirection = { front.x,front.y,front.z,0 };
	data.cameraPosition = { cameraPos.x,cameraPos.y,cameraPos.z,0 };
	immediateContext->UpdateSubresource(constantBuffers[0].Get(), 0, 0, &data, 0, 0);
	immediateContext->VSSetConstantBuffers(1, 1, constantBuffers[0].GetAddressOf());
	immediateContext->PSSetConstantBuffers(1, 1, constantBuffers[0].GetAddressOf());
	
	//2D
	{
		//深度ステートオブジェクトセット
		immediateContext->OMSetDepthStencilState(setting2DDepthStencilState, 1);
		//ラスタライザステートをセット
		immediateContext->RSSetState(setting2DRasterizerState);

	/*	sprites[0].get()->Render(immediateContext.Get(),
	0.0f,0.0f,1280.0f,720.0f,
	spriteColors[0], spriteColors[1], spriteColors[2], spriteColors[3],
	0);*/

		/*sprites[1].get()->Render(immediateContext.Get(),
			700.0f, 200.0f, 200.0f, 200.0f,
			spriteColors[0], spriteColors[1], spriteColors[2], spriteColors[3],
			45,
			0, 0, 140.0f, 240.0f);*/
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

		//prites[2]->Textout(immediateContext.Get(), "FULL SCREEN : alt + enter",0,0,30,30,1,1,1,1);
	}


	//3D
	{
		//深度ステートオブジェクトセット
		immediateContext->OMSetDepthStencilState(setting3DDepthStencilState, 1);
		//ラスタライザステートをセット
		immediateContext->RSSetState(setting3DRasterizerState);

		//geometricPrimitive[0]->Render(immediateContext.Get());
		//geometricPrimitive[1]->Render(immediateContext.Get());

		//staticMeshes[0]->Render(immediateContext.Get());
		//staticMeshes[1]->Render(immediateContext.Get());

		skinnedMeshes[0]->Render(immediateContext.Get());

#ifdef _DEBUG
		immediateContext->RSSetState(rasterizerStates[1].Get());
		//staticMeshes[0]->BoundingBoxRender(immediateContext.Get());
		//staticMeshes[1]->BoundingBoxRender(immediateContext.Get());
#endif // _DEBUG

	}

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

void Framework::SetImguiStyle()
{
	ImGuiStyle* style = &ImGui::GetStyle();
	style->WindowPadding = ImVec2(15, 15);
	style->WindowRounding = 5.0f;
	style->FramePadding = ImVec2(5, 5);
	style->FrameRounding = 4.0f;
	style->ItemSpacing = ImVec2(12, 8);
	style->ItemInnerSpacing = ImVec2(8, 6);
	style->IndentSpacing = 25.0f;
	style->ScrollbarSize = 15.0f;
	style->ScrollbarRounding = 9.0f;
	style->GrabMinSize = 5.0f;
	style->GrabRounding = 3.0f;

	ImGui::StyleColorsLight(style);
	style->Alpha = 0.7f;

	/*ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.94f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.37f, 0.14f, 0.14f, 0.67f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.39f, 0.20f, 0.20f, 0.67f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.48f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.48f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.56f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 0.19f, 0.19f, 0.40f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.89f, 0.00f, 0.19f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(1.00f, 0.19f, 0.19f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.80f, 0.17f, 0.00f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.89f, 0.00f, 0.19f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.33f, 0.35f, 0.36f, 0.53f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.76f, 0.28f, 0.44f, 0.67f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.47f, 0.47f, 0.47f, 0.67f);
	colors[ImGuiCol_Separator] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.85f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
	colors[ImGuiCol_Tab] = ImVec4(0.07f, 0.07f, 0.07f, 0.51f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.86f, 0.23f, 0.43f, 0.67f);
	colors[ImGuiCol_TabActive] = ImVec4(0.19f, 0.19f, 0.19f, 0.57f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.05f, 0.05f, 0.05f, 0.90f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13f, 0.13f, 0.13f, 0.74f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);*/

	//フォント設定
	ImGuiIO& io = ImGui::GetIO();
	//io.Fonts->AddFontFromFileTTF("./imgui/Fonts/Ruda-Bold.ttf", 12);
	//io.Fonts->AddFontFromFileTTF("./imgui/Fonts/Ruda-Bold.ttf", 10);
	io.Fonts->AddFontFromFileTTF("./imgui/Fonts/Ruda-Bold.ttf", 14);
	//io.Fonts->AddFontFromFileTTF("./imgui/Fonts/Ruda-Bold.ttf", 18);

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

void Framework::DrawDebug()
{
#ifdef USE_IMGUI
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Framework"))
		{
			if (ImGui::TreeNode("Camera"))
			{
				ImGui::DragFloat3("position", &cameraPos.x, 0.1f);
				ImGui::DragFloat3("angle", &cameraAngle.x, 0.01f);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Light"))
			{
				ImGui::DragFloat3("angle", &lightAngle.x, 0.01f);
				ImGui::TreePop();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("2D"))
		{
			if (ImGui::TreeNode("SpriteColor"))
			{
				ImGui::ColorPicker4("color", spriteColors, ImGuiColorEditFlags_::ImGuiColorEditFlags_AlphaBar);
				ImGui::TreePop();
			}

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


			static bool selectRFlag[3] = { true,false,false };
			if (ImGui::BeginMenu("RasterizerState"))
			{
				if (ImGui::MenuItem("Solid", "", selectRFlag[0]))
				{
					setting3DRasterizerState = rasterizerStates[0].Get();
					selectRFlag[0] = true; selectRFlag[1] = false; selectRFlag[2] = false;
				}
				if (ImGui::MenuItem("Wireframe", "", selectRFlag[1]))
				{
					setting3DRasterizerState = rasterizerStates[1].Get();
					selectRFlag[0] = false; selectRFlag[1] = true; selectRFlag[2] = false;
				}
				if (ImGui::MenuItem("Wireframe Culling Off", "", selectRFlag[2]))
				{
					setting3DRasterizerState = rasterizerStates[2].Get();
					selectRFlag[0] = false; selectRFlag[1] = false; selectRFlag[2] = true;
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
	
	//ImGui::End();
	geometricPrimitive[0]->DrawDebug();
	geometricPrimitive[1]->DrawDebug();

	staticMeshes[0]->DrawDebug();
	staticMeshes[1]->DrawDebug();

	skinnedMeshes[0]->DrawDebug();

#endif
}
