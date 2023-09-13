#include "TitleScene.h"
#include "GameScene.h"
#include "BaseColorController.h"
#include "Fade.h"
#include "Player.h"
#include "AudioManager.h"

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
	sCursor = std::make_unique<Regal::Resource::Sprite>(graphics.GetDevice(), L"./Resources/Images/Cursor.png");
	sTutrial = std::make_unique<Regal::Resource::Sprite>(graphics.GetDevice(), L"./Resources/Images/Tutrial.png");

	BGParticles = std::make_unique<Regal::Graphics::Particles>(graphics.GetDevice(), 2000);
	popEffect = std::make_unique<PopEffect>(200);
	//Fade::Instance().Initialize();
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
	sCursor->color[0] = color.x;
	sCursor->color[1] = color.y;
	sCursor->color[2] = color.z;

	popEffect->SetColor(color);

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
	/*if (!once)
	{
		goto skip;
		once = true;
	}
	Fade::Instance().Initialize();
	skip : */

	BGParticles->Integrate(Regal::Graphics::Graphics::Instance().GetDeviceContext(), elapsedTime);

	popEffect->Update(elapsedTime);

	if (!isDecide)
	{
		switch (cursorState)
		{
		case PLAY:
			if (Player::MoveDownButton())
			{
				AudioManager::Instance().Play(AudioManager::CURSOR_MOVE);
				cursorState++;
			}
			break;
		case TUTORIAL:
			if (Player::MoveDownButton())
			{
				AudioManager::Instance().Play(AudioManager::CURSOR_MOVE);
				cursorState++;
			}
			if (Player::MoveUpButton())
			{
				AudioManager::Instance().Play(AudioManager::CURSOR_MOVE);
				cursorState--;
			}
			break;
		case EXIT:
			if (Player::MoveUpButton())
			{
				AudioManager::Instance().Play(AudioManager::CURSOR_MOVE);
				cursorState--;
			}
			break;
		}

		if (Player::SelectButton())
		{
			AudioManager::Instance().Play(AudioManager::DECIDE);
			isDecide = true;
			switch (cursorState)
			{
			case PLAY:
				popEffect->Play(DirectX::XMFLOAT3(-12.3f, 33.4f, 0));
				break;
			case TUTORIAL:
				//27.7 20.6
				popEffect->Play(DirectX::XMFLOAT3(-27.7f, 20.6f, 0));
				break;
			}
		}
	}
	else
	{
		//何かが選択された
		switch (cursorState)
		{
		case PLAY:
			Regal::Scene::SceneManager::Instance().ChangeScene(new GameScene);
			//-12.3 33.4
			break;
		case TUTORIAL:
			openTutrial = true;
			if (Player::SelectButton())
			{
				openTutrial = false;
				isDecide = false;
			}
			//27.7 20.6
			break;
		case EXIT:
			exit(0);
			//-12.3 7.5
			break;
		}
	}
	
	//Fade::Instance().Update(elapsedTime);

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

		DirectX::XMFLOAT2 sPos;
		switch (cursorState)
		{
		case PLAY:sPos = {512,405}; break;
		case TUTORIAL:sPos = { 400,492 };break;
		case EXIT:sPos = { 512,585 };break;
		}
		sCursorPos = sPos;

		sCursor->_Render(graphics.GetDeviceContext(), sCursorPos.x, sCursorPos.y,	
			64.0f, 64.0f, 
			0.0f, 0.0f, 128.0f, 128.0f, 0);

		if (openTutrial)
		{
			sTutrial->Render(graphics.GetDeviceContext(), 0, 0,
				graphics.GetScreenWidth(), graphics.GetScreenHeight(), 0);
		}
		//Fade::Instance().Render(immediateContext);

	}

	//パーティクル
	graphics.SetStates(Graphics::ZT_ON_ZW_ON, Graphics::CULL_NONE, Graphics::ALPHA);
	immediateContext->GSSetConstantBuffers(1, 1, graphics.GetShader()->GetSceneConstanceBuffer().GetAddressOf());
	BGParticles->Render(graphics.GetDeviceContext());

	popEffect->Render();


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

	popEffect->DrawDebug();

	sprite->DrawDebug();

	ImGui::DragFloat2("Cursor Pos", &sCursorPos.x);
}

void TitleScene::PostEffectDrawDebug()
{
	bloomer->DrawDebug();

	BGParticles->DrawDebug();
}
