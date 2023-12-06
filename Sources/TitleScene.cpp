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

	//sprite = std::make_unique<Regal::Resource::Sprite>(graphics.GetDevice(), L"./Resources/Images/GameTitle.png");
	sTitle_0 = std::make_unique<Regal::Resource::Sprite>(graphics.GetDevice(), L"./Resources/Images/Game/UI/title.png");
	sTitle_1 = std::make_unique<Regal::Resource::Sprite>(graphics.GetDevice(), L"./Resources/Images/Game/UI/titleUi_1.png");
	sTitle_play = std::make_unique<Regal::Resource::Sprite>(graphics.GetDevice(), L"./Resources/Images/Game/UI/titleUi_play.png");
	sTitle_practice = std::make_unique<Regal::Resource::Sprite>(graphics.GetDevice(), L"./Resources/Images/Game/UI/titleUi_practice.png");
	sTitle_backToTitle = std::make_unique<Regal::Resource::Sprite>(graphics.GetDevice(), L"./Resources/Images/Game/UI/titleUi_backToTitle.png");
	sCursor = std::make_unique<Regal::Resource::Sprite>(graphics.GetDevice(), L"./Resources/Images/Cursor.png");
	sTutrial = std::make_unique<Regal::Resource::Sprite>(graphics.GetDevice(), L"./Resources/Images/Tutrial.png");

	BGParticles = std::make_unique<Regal::Graphics::Particles>(graphics.GetDevice(), 2000);
	popEffect = std::make_unique<PopEffect>(200);
	//Fade::Instance().Initialize();
}

void TitleScene::Initialize()
{
	//sprite->SetColor(0, 1, 0.5f, 1);
	auto color = BaseColorController::GetRundomBrightColor();
	sTitle_0->SetColor(color.x,color.y,color.z,color.w);
	sTitle_1->SetColor(color);
	bloomer->bloomExtractionThreshold = 0;
	bloomer->bloomIntensity = 0.5f;

	BGParticles->color = color;
	BGParticles->Initialize(Regal::Graphics::Graphics::Instance().GetDeviceContext(), 0);
	sCursor->SetColor(color);

	popEffect->SetColor(color);

	Regal::Game::Camera::Instance().GetTransform()->SetPosition(DirectX::XMFLOAT3(2, 44, -88));
	//2 44 -88

	state = 0;

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


	MenuUpdate(elapsedTime);

	BGParticles->Integrate(Regal::Graphics::Graphics::Instance().GetDeviceContext(), elapsedTime);

	popEffect->Update(elapsedTime);


	timer += elapsedTime;
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

	//パーティクル
	graphics.SetStates(Graphics::ZT_ON_ZW_ON, Graphics::CULL_NONE, Graphics::ALPHA);
	immediateContext->GSSetConstantBuffers(1, 1, graphics.GetShader()->GetSceneConstanceBuffer().GetAddressOf());
	BGParticles->Render(graphics.GetDeviceContext());

	popEffect->Render();

	//2D
	{
		graphics.Set2DStates();

		sTitle_0->Render();
		sTitle_1->Render();

		sCursor->GetSpriteTransform().SetScale(1.0f);
		sCursor->Render();

		if (openTutrial)
		{
			sTutrial->Render();
		}
		//Fade::Instance().Render(immediateContext);

	}

	


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

	sTitle_0->DrawDebug();

	ImGui::DragFloat2("Cursor Pos", &sCursorPos.x);
}

void TitleScene::PostEffectDrawDebug()
{
	bloomer->DrawDebug();

	BGParticles->DrawDebug();
}

void TitleScene::MenuUpdate(float elapsedTime)
{
	switch (state)
	{
	case static_cast<int>(SubScene::TITLE):
	{
		if (Player::SelectButton())
		{
			popEffect->Play(DirectX::XMFLOAT3(2.4f,20,-7.3f));
		}
		float alpha = std::fabsf(cosf(timer) * 0.7f) + 0.3f;
		sTitle_1->SetAlpha(alpha);

		break;
	}
	case static_cast<int>(SubScene::MAIN_MENU):
		MainMenuUpdate(elapsedTime);
		break;
	}
}

void TitleScene::MainMenuUpdate(float elapsedTime)
{
	if (!isDecide)
	{
		switch (cursorState)
		{
		case static_cast<int>(SelectMenu::PLAY):
			if (Player::MoveDownButton())
			{
				AudioManager::Instance().Play(AudioManager::CURSOR_MOVE);
				cursorState++;
			}
			break;
		case static_cast<int>(SelectMenu::PRACTICE):
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
		case static_cast<int>(SelectMenu::EXIT):
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
			case static_cast<int>(SelectMenu::PLAY):
				popEffect->Play(DirectX::XMFLOAT3(-12.3f, 33.4f, 0));
				break;
			case static_cast<int>(SelectMenu::PRACTICE):
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
		case static_cast<int>(SelectMenu::PLAY):
			Regal::Scene::SceneManager::Instance().ChangeScene(new GameScene);
			//-12.3 33.4
			break;
		case static_cast<int>(SelectMenu::PRACTICE):
			openTutrial = true;
			if (Player::SelectButton())
			{
				openTutrial = false;
				isDecide = false;
			}
			//27.7 20.6
			break;
		case static_cast<int>(SelectMenu::EXIT):
			exit(0);
			//-12.3 7.5
			break;
		}
	}

	DirectX::XMFLOAT2 sPos;
	switch (cursorState)
	{
	case static_cast<int>(SelectMenu::PLAY):sPos = { 512,405 }; break;
	case static_cast<int>(SelectMenu::PRACTICE):sPos = { 400,492 }; break;
	case static_cast<int>(SelectMenu::EXIT):sPos = { 512,585 }; break;
	}
	sCursorPos = sPos;

	//Fade::Instance().Update(elapsedTime);

	if (Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::F1))
	{
		Regal::Scene::SceneManager::Instance().ChangeScene(new GameScene);
	}
}
