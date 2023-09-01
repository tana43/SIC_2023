#include "framework.h"

//オフスクリーンレンダリング無効
//#define DISABLE_OFFSCREENRENDERING 

using namespace Regal::Resource;
using namespace Regal::Graphics;


Framework::Framework(HWND hwnd,BOOL fullscreen) : 
	hwnd(hwnd),
	graphics(hwnd,fullscreen)
{
	HRESULT hr{S_OK};

	auto device{ Graphics::Instance().GetDevice() };

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(SceneConstants);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffers[0].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	//抽出輝度成分の輝度の閾値を制御するためのバッファ
	bufferDesc.ByteWidth = sizeof(parametricConstants);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffers[1].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

bool Framework::Initialize()
{
	Graphics& graphics{ Graphics::Instance() };

	//各リソースクラスの生成
	//一応残してるだけ、閉じてていい
#if 0 
	sprites[0] = std::make_unique<Sprite>(device.Get(), L"./Resources/cyberpunk.jpg");
	spritesBatches[0] = std::make_unique<SpriteBatch>(device.Get(), L"./Resources/screenshot.jpg", 1);
	geometricPrimitive[1] = std::make_unique<GeometricPrimitive>(device.Get(),
		GeometricPrimitive::MeshType::Cylinder, DirectX::XMFLOAT3(-1.5f, 0, 0), DirectX::XMFLOAT4(0.1f, 0.8f, 0.2f, 1.0f));
	staticMeshes[0] = std::make_unique<StaticMesh>(device.Get(), L"./Resources/F-14A_Tomcat/F-14A_Tomcat.obj", true, DirectX::XMFLOAT3(1.5f, 0, 0));
	skinnedMeshes[0] = std::make_unique<SkinnedMesh>(device.Get(), "./Resources/cube.004.fbx", true);
	skinnedMeshes[0] = std::make_unique<SkinnedMesh>(device.Get(), "./Resources/AimTest/MNK_mesh.fbx");
	skinnedMeshes[0]->AppendAnimations("./Resources/AimTest/Aim_Space.fbx", 0);
	gltfModels[0] = std::make_unique<GltfModel>(device.Get(),
		//"./Resources/glTF-Sample-Models-master/2.0/2CylinderEngine/glTF/2CylinderEngine.gltf"
		//"./Resources/glTF-Sample-Models-master/2.0/DamagedHelmet/glTF/DamagedHelmet.gltf"
		//"./Resources/glTF-Sample-Models-master/2.0/Fox/glTF/Fox.gltf"
		//"./Resources/glTF-Sample-Models-master/2.0/CesiumMan/glTF/CesiumMan.gltf"
		//"./Resources/glTF-Sample-Models-master/2.0/BrainStem/glTF/BrainStem.gltf"
		"./Resources/deathwing/scene.gltf"
		//"./Resources/cube.glb"
		//"./Resources/crunch.gltf"
		//"./Resources/Crunch/Crunch.gltf"
		//"./Resources/Stage/Showcase.gltf"
	);
#endif // 0

	gltfModels[0] = std::make_unique<GltfModel>(graphics.GetDevice(),
		//"./Resources/glTF-Sample-Models-master/2.0/2CylinderEngine/glTF/2CylinderEngine.gltf"
		//"./Resources/glTF-Sample-Models-master/2.0/DamagedHelmet/glTF/DamagedHelmet.gltf"
		//"./Resources/glTF-Sample-Models-master/2.0/Fox/glTF/Fox.gltf"
		//"./Resources/glTF-Sample-Models-master/2.0/CesiumMan/glTF/CesiumMan.gltf"
		//"./Resources/glTF-Sample-Models-master/2.0/BrainStem/glTF/BrainStem.gltf"
		"./Resources/deathwing/scene.gltf"
		//"./Resources/cube.glb"
		//"./Resources/crunch.gltf"
		//"./Resources/Crunch/Crunch.gltf"
		//"./Resources/Stage/Showcase.gltf"
	);

	framebuffers[0] = std::make_unique<Framebuffer>(graphics.GetDevice(), graphics.GetScreenWidth(), graphics.GetScreenHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT, true);
	framebuffers[1] = std::make_unique<Framebuffer>(graphics.GetDevice(), graphics.GetScreenWidth()/2, graphics.GetScreenHeight()/2, DXGI_FORMAT_R16G16B16A16_FLOAT, false);

	bitBlockTransfer = std::make_unique<FullscreenQuad>(graphics.GetDevice());

	////スカイボックス
	//skyboxSprite = std::make_unique<Sprite>(device.Get(), 
	//	//L"./Resources/SkyBox/envmap_miramar.dds"
	//	//L"./Resources/SkyBox/Sky.png"
	//	//L"./Resources/SkyBox/realistic_bloodborne.jpg"
	//	L"./Resources/SkyBox/realistic_tokyo_night_view.jpg"
	//);
	//skybox = std::make_unique<SkyBox>(device.Get(), skyboxSprite.get());

	////各種ステートオブジェクトセット
	//{
	//	setting2DDepthStencilState = depthStencilStates[3].Get();
	//	setting3DDepthStencilState = depthStencilStates[0].Get();
	//	setting2DRasterizerState = rasterizerStates[0].Get();
	//	setting3DRasterizerState = rasterizerStates[0].Get();
	//}

	Shader::CreatePSFromCso(graphics.GetDevice(), "./Resources/Shader/LuminanceExtractionPS.cso", pixelShaders[0].GetAddressOf());
	Shader::CreatePSFromCso(graphics.GetDevice(), "./Resources/Shader/BlurPS.cso", pixelShaders[1].GetAddressOf());

	D3D11_TEXTURE2D_DESC texture2dDesc;
	LoadTextureFromFile(graphics.GetDevice(), L"./Resources/environments/sunset_jhbcentral_4k/sunset_jhbcentral_4k.dds", 
		shaderResourceViews[0].GetAddressOf(), &texture2dDesc);
	LoadTextureFromFile(graphics.GetDevice(), L"./Resources/environments/sunset_jhbcentral_4k/diffuse_iem.dds",
		shaderResourceViews[1].GetAddressOf(), &texture2dDesc);
	LoadTextureFromFile(graphics.GetDevice(), L"./Resources/environments/sunset_jhbcentral_4k/specular_pmrem.dds",
		shaderResourceViews[2].GetAddressOf(), &texture2dDesc);
	LoadTextureFromFile(graphics.GetDevice(), L"./Resources/environments/lut_ggx.dds",
		shaderResourceViews[3].GetAddressOf(), &texture2dDesc);

	bloomer = std::make_unique<Bloom>(graphics.GetDevice(), graphics.GetScreenWidth(), graphics.GetScreenHeight());
	Shader::CreatePSFromCso(graphics.GetDevice(), "./Resources/Shader/FinalPassPS.cso", pixelShaders[0].ReleaseAndGetAddressOf());

	particles = std::make_unique<decltype(particles)::element_type>(graphics.GetDevice(), 1000);
	particles->Initialize(graphics.GetDeviceContext(), 0);

	return true;
}

void Framework::Update(float elapsed_time/*Elapsed seconds from last frame*/)
{
#ifdef USE_IMGUI
	// ImGui更新
	IMGUI_CTRL_CLEAR_FRAME();
#endif

	Graphics& graphics{ Graphics::Instance() };

	if (GetAsyncKeyState(VK_RETURN) & 1 && GetAsyncKeyState(VK_MENU) & 1)
	{
		graphics.FullscreenState(!graphics.fullscreenMode);
	}

	//スペースでパーティクルリセット
	if (GetAsyncKeyState(' ') & 0x8000)
	{
		particles->Initialize(graphics.GetDeviceContext(), 0);
	}
	particles->Integrate(graphics.GetDeviceContext(), elapsed_time);

#if _DEBUG
	DrawDebug();

	//ImGui::ShowDemoWindow();

#endif // _DEBUG
	
}

void Framework::Render(float elapsedTime/*Elapsed seconds from last frame*/)
{
	HRESULT hr{ S_OK };

	/*ID3D11RenderTargetView* nullRenderTargetViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
	immediateContext->OMSetRenderTargets(_countof(nullRenderTargetViews), nullRenderTargetViews, 0);
	ID3D11ShaderResourceView* nullShaderResourcesViews[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	immediateContext->VSSetShaderResources(0, _countof(nullShaderResourcesViews), nullShaderResourcesViews);
	immediateContext->PSSetShaderResources(0, _countof(nullShaderResourcesViews), nullShaderResourcesViews);


	immediateContext->ClearRenderTargetView(renderTargetView.Get(), color);
	immediateContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	immediateContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());*/

	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* immediateContext = graphics.GetDeviceContext();
	ID3D11RenderTargetView* rtv = graphics.GetRenderTargetView();
	ID3D11DepthStencilView* dsv = graphics.GetDepthStencilView();

	//画面クリア&レンダーターゲット設定
	immediateContext->ClearRenderTargetView(rtv, color);
	immediateContext->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	immediateContext->OMSetRenderTargets(1, &rtv, dsv);

	//テクスチャバインド
	immediateContext->PSSetShaderResources(32, 1, shaderResourceViews[0].GetAddressOf());
	immediateContext->PSSetShaderResources(33, 1, shaderResourceViews[1].GetAddressOf());
	immediateContext->PSSetShaderResources(34, 1, shaderResourceViews[2].GetAddressOf());
	immediateContext->PSSetShaderResources(35, 1, shaderResourceViews[3].GetAddressOf());

	//サンプラーステートオブジェクトをバインド
	/*immediateContext->PSSetSamplers(0, 1, samplerStates[0].GetAddressOf());
	immediateContext->PSSetSamplers(1, 1, samplerStates[1].GetAddressOf());
	immediateContext->PSSetSamplers(2, 1, samplerStates[2].GetAddressOf());*/
	graphics.BindSamplersState();

	//ビュー・プロジェクション交換行列を計算
	D3D11_VIEWPORT viewport;
	UINT numViewports{ 1 };
	immediateContext->RSGetViewports(&numViewports, &viewport);

	float aspectRaito{ viewport.Width / viewport.Height };
	DirectX::XMMATRIX P{ DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(cameraFov),aspectRaito,0.1f,cameraFar) };

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
	//DirectX::XMStoreFloat4x4(&data.inverseViewProjection, DirectX::XMMatrixInverse(nullptr,V * P));
	
	
	immediateContext->UpdateSubresource(constantBuffers[1].Get(), 0, 0, &parametricConstants, 0, 0);
	immediateContext->PSSetConstantBuffers(2, 1, constantBuffers[1].GetAddressOf());
#ifndef DISABLE_OFFSCREENRENDERING
	framebuffers[0]->Clear(immediateContext,color[0], color[1], color[2], color[3]);
	framebuffers[0]->Activate(immediateContext);
#endif // !ENABLE_OFFSCREENRENDERING

	//背景
	//skybox->Render(immediateContext.Get(), V, P);

	//背景で使うシーン用バッファーに上書きされないように背景描画後にバッファー更新
	immediateContext->UpdateSubresource(constantBuffers[0].Get(), 0, 0, &data, 0, 0);
	immediateContext->VSSetConstantBuffers(1, 1, constantBuffers[0].GetAddressOf());
	immediateContext->PSSetConstantBuffers(1, 1, constantBuffers[0].GetAddressOf());
	//immediateContext->RSSetState(rasterizerStates[4].Get());


	//2D
	{
		graphics.Set2DStates();
	}

	//パーティクル
	graphics.SetStates(Graphics::ZT_ON_ZW_ON,Graphics::CULL_NONE,Graphics::ALPHA);
	immediateContext->GSSetConstantBuffers(1, 1, constantBuffers[0].GetAddressOf());
	particles->Render(immediateContext);

	//3D
	{
		graphics.Set3DStates();

		static std::vector<GltfModel::Node> animatedNodes{gltfModels[0]->nodes};
		static float time{ 0 };
		gltfModels[0]->Animate(0, time += elapsedTime, animatedNodes);
		gltfModels[0]->Render(graphics.GetDeviceContext(), animatedNodes);

		//描画エンジンの課題範囲での描画、閉じてていい
#if 0


#if 0
		int clipIndex{ 0 };
		int frameIndex{ 0 };
		static float animationTick{ 0 };

		Animation& animation{ skinnedMeshes[0]->animationClips.at(clipIndex) };
		frameIndex = static_cast<int>(animationTick * animation.samplingRate);
		if (frameIndex > animation.sequence.size() - 1)
		{
			frameIndex = 0;
			animationTick = 0;
		}
		else
		{
			animationTick += elapsedTime;
		}

		Animation::Keyframe& keyframe{animation.sequence.at(frameIndex)};

#else
		//アニメーションブレンドのサンプル
		/*Animation::Keyframe keyframe;
		const Animation::Keyframe* keyframes[2]{
			&skinnedMeshes[0]->animationClips.at(0).sequence.at(40),
			&skinnedMeshes[0]->animationClips.at(0).sequence.at(80)
		};
		skinnedMeshes[0]->BlendAnimations(keyframes, blendAnimation, keyframe);
		skinnedMeshes[0]->UpdateAnimation(keyframe);*/

#endif // 0

		skinnedMeshes[0]->Render(immediateContext.Get(), &keyframe);

		static std::vector<GltfModel::Node> animatedNodes{gltfModels[0]->nodes};
		static float time{ 0 };
		gltfModels[0]->Animate(0, time += elapsedTime, animatedNodes);
		gltfModels[0]->Render(immediateContext.Get(),animatedNodes);
#endif // 0

#ifndef DISABLE_OFFSCREENRENDERING
	framebuffers[0]->Deactivate(immediateContext);
#endif // !DISABLE_OFFSCREENRENDERING

#ifdef _DEBUG
		//immediateContext->RSSetState(rasterizerStates[1].Get());
		//staticMeshes[0]->BoundingBoxRender(immediateContext.Get());
		//staticMeshes[1]->BoundingBoxRender(immediateContext.Get());
#endif // _DEBUG
	}
	

	//ブルーム
	{
		bloomer->Make(immediateContext, framebuffers[0]->shaderResourceViews[0].Get());
		graphics.SetStates(Graphics::ZT_OFF_ZW_OFF, Graphics::CULL_NONE, Graphics::ALPHA);
		ID3D11ShaderResourceView* shaderResourceViews[] =
		{
			framebuffers[0]->shaderResourceViews[0].Get(),
			bloomer->ShaderResourceView(),
		};
		bitBlockTransfer->Bilt(immediateContext, shaderResourceViews, 0, 2, pixelShaders[0].Get());
	}

#ifdef USE_IMGUI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif

	UINT syncInterval{ graphics.vsync ? 1U : 0U };
	UINT flags = (graphics.tearingSupported && !graphics.fullscreenMode && !graphics.vsync) ? DXGI_PRESENT_ALLOW_TEARING : 0;
	hr = graphics.GetSwapChain()->Present(syncInterval, flags);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

bool Framework::Uninitialize()
{
	return true;
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
	io.Fonts->AddFontFromFileTTF("./External/imgui/Fonts/Ruda-Bold.ttf", 14);
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
				ImGui::DragFloat3("Angle", &cameraAngle.x, 0.01f);
				ImGui::DragFloat("Fov", &cameraFov,0.1f);
				ImGui::DragFloat("Far", &cameraFar,1.0f);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Light"))
			{
				ImGui::DragFloat3("angle", &lightAngle.x, 0.01f);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("ClearColor"))
			{
				ImGui::ColorPicker4("color",color);
				ImGui::TreePop();
			}
			ImGui::EndMenu();
		}

		/*if (ImGui::BeginMenu("2D"))
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

			ImGui::DragFloat("BoneTranslation",&boneTranslationX);

			ImGui::SliderFloat("BlendAnimation",&blendAnimation,0.0f,1.0f);

			ImGui::EndMenu();
		}*/

		if (ImGui::BeginMenu("PostEffect"))
		{
			/*ImGui::SliderFloat("ExtractionThreshold", &parametricConstants.extractionThreshold,0,1);
			ImGui::SliderFloat("GaussianSigma", &parametricConstants.gaussianSigma,0.001f,10);
			ImGui::SliderFloat("BloomIntensity", &parametricConstants.bloomIntensity,0,5);
			ImGui::SliderFloat("Exposure", &parametricConstants.exposure, 0.0f, 2.2f);*/

			bloomer->DrawDebug();

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
	
	//ImGui::End();
	/*geometricPrimitive[0]->DrawDebug();
	geometricPrimitive[1]->DrawDebug();

	staticMeshes[0]->DrawDebug();
	staticMeshes[1]->DrawDebug();*/

	//skinnedMeshes[0]->DrawDebug();

	//gltfModels[0]->DrawDebug();

	particles->DrawDebug();

#endif
}
