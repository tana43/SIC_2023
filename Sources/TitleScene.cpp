#include "TitleScene.h"
#include "GameScene.h"
#include "BaseColorController.h"

void TitleScene::CreateResource()
{
	auto& graphics{ Regal::Graphics::Graphics::Instance() };

	framebuffer = std::make_unique<Regal::Graphics::Framebuffer>(
		graphics.GetDevice(), graphics.GetScreenWidth(), graphics.GetScreenHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT, true);
	bitBlockTransfer = std::make_unique < Regal::Graphics::FullscreenQuad>(graphics.GetDevice());


	Regal::Resource::Shader::CreatePSFromCso(graphics.GetDevice(), "./Resources/Shader/LuminanceExtractionPS.cso", LEPixelShader.GetAddressOf());

	bloomer = std::make_unique<Regal::Graphics::Bloom>(graphics.GetDevice(), graphics.GetScreenWidth(), graphics.GetScreenHeight());
	Regal::Resource::Shader::CreatePSFromCso(graphics.GetDevice(), "./Resources/Shader/FinalPassPS.cso", LEPixelShader.ReleaseAndGetAddressOf());

	sprite = std::make_unique<Regal::Resource::Sprite>(graphics.GetDevice(), L"./Resources/Images/GameTitle.png");

	BGParticles = std::make_unique<Regal::Graphics::Particles>(graphics.GetDevice(), 2000);
}

void TitleScene::Initialize()
{
	//sprite->SetColor(0, 1, 0.5f, 1);
	auto color = BaseColorController::RundomBrightColor();
	sprite->SetColor(color.x,color.y,color.z,color.w);
	bloomer->bloomExtractionThreshold = 0;
	bloomer->bloomIntensity = 0.5f;

	BGParticles->color = color;
	BGParticles->Initialize(Regal::Graphics::Graphics::Instance().GetDeviceContext(), 0);

	Regal::Game::Camera::Instance().GetTransform()->SetPosition(DirectX::XMFLOAT3(2, 44, -88));
	//2 44 -88
}

void TitleScene::Finalize()
{
}

void TitleScene::Begin()
{
}

void TitleScene::Update(const float& elapsedTime)
{
	BGParticles->Integrate(Regal::Graphics::Graphics::Instance().GetDeviceContext(), elapsedTime);

	if (Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::F1))
	{
		Regal::Scene::SceneManager::Instance().ChangeScene(new GameScene);
	}
}

void TitleScene::End()
{
}

void TitleScene::Render(const float& elapsedTime)
{
	using namespace Regal;

	auto& graphics{ Graphics::Graphics::Instance() };
	auto* immediateContext{ graphics.GetDeviceContext() };

#ifndef DISABLE_OFFSCREENRENDERING
	framebuffer->Clear(immediateContext, clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
	framebuffer->Activate(immediateContext);
#endif // !ENABLE_OFFSCREENRENDERING

	//2D
	{
		graphics.Set2DStates();

		sprite->Render(graphics.GetDeviceContext(),0,0,
			graphics.GetScreenWidth(),graphics.GetScreenHeight(),0);
	}

	//パーティクル
	graphics.SetStates(Graphics::ZT_ON_ZW_ON, Graphics::CULL_NONE, Graphics::ALPHA);
	immediateContext->GSSetConstantBuffers(1, 1, graphics.GetShader()->GetSceneConstanceBuffer().GetAddressOf());
	BGParticles->Render(graphics.GetDeviceContext());

	//3D
	{
		graphics.Set3DStates();

	}
#ifndef DISABLE_OFFSCREENRENDERING
		framebuffer->Deactivate(immediateContext);
#endif // !DISABLE_OFFSCREENRENDERING

	//ブルーム
	{
		bloomer->Make(immediateContext, framebuffer->shaderResourceViews[0].Get());
		graphics.SetStates(Graphics::ZT_OFF_ZW_OFF, Graphics::CULL_NONE, Graphics::ALPHA);
		ID3D11ShaderResourceView* shaderResourceViews[] =
		{
			framebuffer->shaderResourceViews[0].Get(),
			bloomer->ShaderResourceView(),
		};
		bitBlockTransfer->Bilt(immediateContext, shaderResourceViews, 0, 2, LEPixelShader.Get());
	}
}

void TitleScene::DrawDebug()
{
	if (ImGui::BeginMenu("Clear Color"))
	{
		ImGui::ColorEdit4("Color", &clearColor[0]);

		ImGui::EndMenu();
	}


	sprite->DrawDebug();
}

void TitleScene::PostEffectDrawDebug()
{
	bloomer->DrawDebug();

	BGParticles->DrawDebug();
}
