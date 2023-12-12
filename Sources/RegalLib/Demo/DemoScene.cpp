#include "DemoScene.h"

namespace Regal::Demo
{
    void DemoScene::CreateResource()
    {
		auto& graphics{ Regal::Graphics::Graphics::Instance() };

		particles = std::make_unique<Regal::Graphics::Particles>(graphics.GetDevice(), 1000);

		framebuffer = std::make_unique<Regal::Graphics::Framebuffer>(
			graphics.GetDevice(), graphics.GetScreenWidth(), graphics.GetScreenHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT, true);
		bitBlockTransfer = std::make_unique < Regal::Graphics::FullscreenQuad>(graphics.GetDevice());


		Regal::Resource::Shader::CreatePSFromCso(graphics.GetDevice(), "./Resources/Shader/LuminanceExtractionPS.cso", LEPixelShader.GetAddressOf());

		bloomer = std::make_unique<Regal::Graphics::Bloom>(graphics.GetDevice(), graphics.GetScreenWidth(), graphics.GetScreenHeight());
		Regal::Resource::Shader::CreatePSFromCso(graphics.GetDevice(), "./Resources/Shader/FinalPassPS.cso", LEPixelShader.ReleaseAndGetAddressOf());

		model = std::make_unique<Regal::Model::StaticModel>("./Resources/Models/LuminousCube04.fbx");
		//model = std::make_unique<Regal::Model::StaticModel>("./Resources/Models/LuminousHexagon01.fbx");
		//model = std::make_unique<Regal::Model::StaticModel>("./Resources/Models/TESTidle.fbx");

		/*for (size_t i = 0; i < 500; i++)
		{
			hexagons[i] = std::make_unique<Regal::Model::StaticModel>("./Resources/Models/LuminousHexagon01.fbx");
			hexagons[i]->GetTransform()->SetPositionX(static_cast<float>(i * 2));
		}*/

		sTitle_0 = std::make_unique<Regal::Resource::Sprite>(graphics.GetDevice(), L"./Resources/Images/Demo.png");

		demoEffect = std::make_unique<DemoEffect>(size_t(1000));
    }

    void DemoScene::Initialize()
    {
		particles->Initialize(Regal::Graphics::Graphics::Instance().GetDeviceContext(), 0);
		sTitle_0->SetColor(1, 1, 1, 0.01f);
    }

    void DemoScene::Finalize()
    {
    }

    void DemoScene::Begin()
    {
    }

    void DemoScene::Update(const float& elapsedTime)
    {
		//スペースでパーティクルリセット
		if (Regal::Input::Keyboard::Instance().GetKeyDown(DirectX::Keyboard::Space))
		{
			particles->Initialize(Regal::Graphics::Graphics::Instance().GetDeviceContext(), 0);
		}

		particles->Integrate(Regal::Graphics::Graphics::Instance().GetDeviceContext(),elapsedTime);

		demoEffect->Update(elapsedTime);
    }

    void DemoScene::End()
    {
    }

    void DemoScene::Render(const float& elapsedTime)
    {
		using namespace Regal;

		auto& graphics{ Graphics::Graphics::Instance() };
		auto* immediateContext{ graphics.GetDeviceContext() };

		//背景
		//skybox->Render(immediateContext.Get(), V, P);


		//背景で使うシーン用バッファーに上書きされないように背景描画後にバッファー更新
		/*immediateContext->UpdateSubresource(constantBuffers[0].Get(), 0, 0, &data, 0, 0);
		immediateContext->VSSetConstantBuffers(1, 1, constantBuffers[0].GetAddressOf());
		immediateContext->PSSetConstantBuffers(1, 1, constantBuffers[0].GetAddressOf());*/
		//immediateContext->RSSetState(rasterizerStates[4].Get());


#ifndef DISABLE_OFFSCREENRENDERING
		framebuffer->Clear(immediateContext, clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
		framebuffer->Activate(immediateContext);
#endif // !ENABLE_OFFSCREENRENDERING

		//graphics.SetStates(Graphics::ZT_ON_ZW_ON, Graphics::SOLID, Graphics::ALPHA);
		

		//2D
		{
			graphics.Set2DStates();

			sTitle_0->Render();

		}

		//パーティクル
		graphics.SetStates(Graphics::ZT_ON_ZW_ON, Graphics::CULL_NONE, Graphics::ALPHA);
		immediateContext->GSSetConstantBuffers(1, 1, graphics.GetShader()->GetSceneConstanceBuffer().GetAddressOf());
		particles->Render(immediateContext);

			demoEffect->Render();

		//3D
		{
			graphics.Set3DStates();

			/*for (size_t i = 0; i < 500; i++)
			{
				hexagons[i]->Render();
			}*/

			model->Render();

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
			gltfModels[0]->Render(immediateContext.Get(), animatedNodes);
#endif // 0
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

	

    void DemoScene::DrawDebug()
    {
		PostEffectDrawDebug();

		if (ImGui::BeginMenu("Clear Color"))
		{
			ImGui::ColorEdit4("Color",&clearColor[0]);


			ImGui::EndMenu();
		}

		particles->DrawDebug();

		model->DrawDebug();

		sTitle_0->DrawDebug();

		demoEffect->DrawDebug();
    }

	void DemoScene::PostEffectDrawDebug()
	{
		if (ImGui::BeginMenu("PostEffect"))
		{
			bloomer->DrawDebug();
			ImGui::EndMenu();
		}
	}
}