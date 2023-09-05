#include "framework.h"
#include "TitleScene.h"

using namespace Regal::Resource;
using namespace Regal::Graphics;
using namespace Regal::Input;
using namespace Regal::Game;
using namespace Regal::Scene;
using namespace Regal::Demo;


Framework::Framework(HWND hwnd,BOOL fullscreen) : 
	hwnd(hwnd),
	graphics(hwnd,fullscreen)
{
}

bool Framework::Initialize()
{
	HRESULT hr{ S_OK };

	//シーン用バッファー作成
	graphics.GetShader()->CreateSceneBuffer(graphics.GetDevice());

	//抽出輝度成分の輝度の閾値を制御するためのバッファ
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(parametricConstants);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	hr = graphics.GetDevice()->CreateBuffer(&bufferDesc, nullptr, constantBuffers[1].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	Graphics& graphics{ Graphics::Instance() };

	Camera::Instance().Initialize();

	//描画エンジン等の各クラスの生成
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

	framebuffers[0] = std::make_unique<Framebuffer>(graphics.GetDevice(), graphics.GetScreenWidth(), graphics.GetScreenHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT, true);
	//framebuffers[1] = std::make_unique<Framebuffer>(graphics.GetDevice(), graphics.GetScreenWidth() / 2, graphics.GetScreenHeight() / 2, DXGI_FORMAT_R16G16B16A16_FLOAT, false);

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

#endif // 0
	
#ifdef DEMO_MODE
	SceneManager::Instance().ChangeScene(new DemoScene);
#else
	SceneManager::Instance().ChangeScene(new TitleScene);
#endif

	return true;
}

void Framework::Update(float elapsedTime/*Elapsed seconds from last frame*/)
{
#ifdef USE_IMGUI
	// ImGui更新
	IMGUI_CTRL_CLEAR_FRAME();
#endif

	Input::Instance().Update();

	Graphics& graphics{ Graphics::Instance() };

	//オフスクリーンレンダリングのスクリーンサイズ変更処理をしていないため
	//途中でフルスクリーンに変更するとエラーが起きるのでコメントアウトしておく
	/*if (GetAsyncKeyState(VK_RETURN) & 1 && GetAsyncKeyState(VK_MENU) & 1)
	{
		graphics.FullscreenState(!graphics.fullscreenMode);
	}*/

	SceneManager::Instance().Update(elapsedTime);

#if _DEBUG
	DrawDebug();

	//ImGui::ShowDemoWindow();

#endif // _DEBUG
	
}

void Framework::Render(float elapsedTime/*Elapsed seconds from last frame*/)
{
	HRESULT hr{ S_OK };

	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* immediateContext = graphics.GetDeviceContext();
	ID3D11RenderTargetView* rtv = graphics.GetRenderTargetView();
	ID3D11DepthStencilView* dsv = graphics.GetDepthStencilView();

	//画面クリア&レンダーターゲット設定
	immediateContext->ClearRenderTargetView(rtv, color);
	immediateContext->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	immediateContext->OMSetRenderTargets(1, &rtv, dsv);

	//PBRテクスチャバインド
	//immediateContext->PSSetShaderResources(32, 1, shaderResourceViews[0].GetAddressOf());
	//immediateContext->PSSetShaderResources(33, 1, shaderResourceViews[1].GetAddressOf());
	//immediateContext->PSSetShaderResources(34, 1, shaderResourceViews[2].GetAddressOf());
	//immediateContext->PSSetShaderResources(35, 1, shaderResourceViews[3].GetAddressOf());

	//サンプラーステートオブジェクトをバインド
	graphics.BindSamplersState();

	//シーン用バッファ更新
	graphics.GetShader()->UpdateSceneConstants(immediateContext);
	
	SceneManager::Instance().Render(elapsedTime);

#ifdef _DEBUG
		
#endif // _DEBUG

	IMGUI_CTRL_DISPLAY();

	UINT syncInterval{ graphics.vsync ? 1U : 0U };
	UINT flags = (graphics.tearingSupported && !graphics.fullscreenMode && !graphics.vsync) ? DXGI_PRESENT_ALLOW_TEARING : 0;
	hr = graphics.GetSwapChain()->Present(syncInterval, flags);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

bool Framework::Uninitialize()
{
	SceneManager::Instance().Clear();

	return true;
}

Framework::~Framework()
{

}

void Framework::DrawDebug()
{
#ifdef USE_IMGUI
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Framework"))
		{

			Camera::Instance().DrawDebug();

			if (ImGui::BeginMenu("ClearColor"))
			{
				ImGui::ColorPicker4("color",color);
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		graphics.DrawDebug();

		SceneManager::Instance().DrawDebug();

		ImGui::EndMainMenuBar();
	}
	

#endif
}
