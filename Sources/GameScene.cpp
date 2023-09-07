#include "GameScene.h"
#include "TitleScene.h"
#include "PuzzleFrame.h"

void GameScene::CreateResource()
{
	auto& graphics{ Regal::Graphics::Graphics::Instance() };

	framebuffer = std::make_unique<Regal::Graphics::Framebuffer>(
		graphics.GetDevice(), graphics.GetScreenWidth(), graphics.GetScreenHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT, true);
	bitBlockTransfer = std::make_unique < Regal::Graphics::FullscreenQuad>(graphics.GetDevice());


	Regal::Resource::Shader::CreatePSFromCso(graphics.GetDevice(), "./Resources/Shader/LuminanceExtractionPS.cso", LEPixelShader.GetAddressOf());

	bloomer = std::make_unique<Regal::Graphics::Bloom>(graphics.GetDevice(), graphics.GetScreenWidth(), graphics.GetScreenHeight());
	Regal::Resource::Shader::CreatePSFromCso(graphics.GetDevice(), "./Resources/Shader/FinalPassPS.cso", LEPixelShader.ReleaseAndGetAddressOf());

#if _DEBUG
	sprite = std::make_unique<Regal::Resource::Sprite>(graphics.GetDevice(), L"./Resources/Images/Game.png");
#endif // _DEBUG


	hexagon.CreateResource();

	PuzzleFrame::Instance().CreateResource();
}

void GameScene::Initialize()
{
#if _DEBUG
	sprite->SetColor(1, 1, 1, 0.3f);
#endif // _DEBUG

	PuzzleFrame::Instance().Initialize();

	hexagon.Initialize();

}

void GameScene::Finalize()
{
}

void GameScene::Begin()
{
}

void GameScene::Update(const float& elapsedTime)
{
	if (Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::F1))
	{
		Regal::Scene::SceneManager::Instance().ChangeScene(new TitleScene);
	}

	hexagon.Update(elapsedTime);

	PuzzleFrame::Instance().Update(elapsedTime);
}

void GameScene::End()
{
}

void GameScene::Render(const float& elapsedTime)
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

#if _DEBUG
		sprite->Render(graphics.GetDeviceContext(), 0, 0,
			graphics.GetScreenWidth(), graphics.GetScreenHeight(), 0);
#endif // _DEBUG
	}

	//パーティクル
	graphics.SetStates(Graphics::ZT_ON_ZW_ON, Graphics::CULL_NONE, Graphics::ALPHA);
	immediateContext->GSSetConstantBuffers(1, 1, graphics.GetShader()->GetSceneConstanceBuffer().GetAddressOf());

	//3D
	{
		graphics.Set3DStates();

		hexagon.Render();


		PuzzleFrame::Instance().Render();
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

void GameScene::DrawDebug()
{
	if (ImGui::BeginMenu("Clear Color"))
	{
		ImGui::ColorEdit4("Color", &clearColor[0]);

		ImGui::EndMenu();
	}


	sprite->DrawDebug();

	hexagon.DrawDebug();

	PuzzleFrame::Instance().DrawDebug();
}

void GameScene::PostEffectDrawDebug()
{
	bloomer->DrawDebug();
}
