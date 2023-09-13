#include "GameScene.h"
#include "TitleScene.h"
#include "PuzzleFrame.h"
#include "BlockManager.h"
#include "BlockGroupManager.h"
#include "GameManager.h"
#include "EnemyManager.h"
#include "Fade.h"

#define ENABLE_PARTICLE 1

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
	sHealth = std::make_unique<Regal::Resource::Sprite>(graphics.GetDevice(), L"./Resources/Images/Health.png");
	sEnemy = std::make_unique<Regal::Resource::Sprite>(graphics.GetDevice(), L"./Resources/Images/Enemy.png");

	PuzzleFrame::Instance().CreateResource();

#if ENABLE_PARTICLE
	BGParticles = std::make_unique<Regal::Graphics::Particles>(graphics.GetDevice(), 2000);
#endif // ENABLE_PARTICLE

	GameManager::Instance().CreateResource();
}

void GameScene::Initialize()
{
#if _DEBUG
	//sprite->SetColor(1, 1, 1, 0.3f);
	sprite->SetColor(1, 1, 1, 0.0f);
#endif // _DEBUG

	PuzzleFrame::Instance().Initialize();

	BlockManager::Instance().Initialize();

	BlockGroupManager::Instance().Initialize();
	/*for (int i = 0; i < 10; i++)
	{
		BlockManager::Instance().Register(new Block);
	}*/

	EnemyManager::Instance().Initialize();

	GameManager::Instance().Initialize();

	
#if ENABLE_PARTICLE
	BGParticles->color = DirectX::XMFLOAT4(0, 0.3f, 1.0f, 1);
	BGParticles->Initialize(Regal::Graphics::Graphics::Instance().GetDeviceContext(),0);
#endif // ENABLE_PARTICLE


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

	BlockGroupManager::Instance().Update(elapsedTime);

	BlockManager::Instance().Update(elapsedTime);

	PuzzleFrame::Instance().Update(elapsedTime);

	//blockGroup.Update(elapsedTime);

	GameManager::Instance().Update(elapsedTime);

	EnemyManager::Instance().Update(elapsedTime);

#if ENABLE_PARTICLE
	BGParticles->Integrate(Regal::Graphics::Graphics::Instance().GetDeviceContext(), elapsedTime);
#endif // ENABLE_PARTICLE

	Regal::Game::Camera::Instance().Update(elapsedTime);


	if (particleTimer > 10.0f)
	{
		float& particleSize = BGParticles->particleData.particleSize;
		if (particleSize > 0)
		{
			BGParticles->particleData.particleSize -= elapsedTime * 0.3f;
			if (particleSize > 0)BGParticles->SetColor(EnemyManager::Instance().GetEnemy()->GetModel()->GetSkinnedMesh()->GetEmissiveColor());
		}
		else
		{
			particleSize += elapsedTime * 0.3f;
			if (0.08f > particleSize)
			{
				particleTimer = 0;
			}
		}
	}
	particleTimer += elapsedTime;
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

#if ENABLE_PARTICLE
	//パーティクル
	graphics.SetStates(Graphics::ZT_ON_ZW_ON, Graphics::CULL_NONE, Graphics::ALPHA);
	immediateContext->GSSetConstantBuffers(1, 1, graphics.GetShader()->GetSceneConstanceBuffer().GetAddressOf());
	BGParticles->Render(graphics.GetDeviceContext());
#endif // ENABLE_PARTICLE

	//3D
	{
		graphics.Set3DStates();

		BlockManager::Instance().Render();

		PuzzleFrame::Instance().Render();

		GameManager::Instance().Render();

		EnemyManager::Instance().Render();
	}

	//2D
	{
		graphics.Set2DStates();

#if _DEBUG
		sprite->Render(graphics.GetDeviceContext(), 0, 0,
			graphics.GetScreenWidth(), graphics.GetScreenHeight(), 0);
#endif // _DEBUG

		sHealth->Render(graphics.GetDeviceContext(), 0, 0,
			graphics.GetScreenWidth(), graphics.GetScreenHeight(), 0);

		sEnemy->Render(graphics.GetDeviceContext(), 0, 0,
			graphics.GetScreenWidth(), graphics.GetScreenHeight(), 0);

		//Fade::Instance().Render(immediateContext);

		GameManager::Instance().GameClearRender();
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

	BlockManager::Instance().DrawDebug();

	BlockGroupManager::Instance().DrawDebug();

	PuzzleFrame::Instance().DrawDebug();

	blockGroup.DrawDebug();

	GameManager::Instance().DrawDebug();

#if ENABLE_PARTICLE
	BGParticles->DrawDebug();
#endif // ENABLE_PARTICLE

	EnemyManager::Instance().DrawDebug();
}

void GameScene::PostEffectDrawDebug()
{
	bloomer->DrawDebug();
}
