#include "DemoScene.h"
#include "../Graphics/Graphics.h"

namespace Regal::Demo
{
    void DemoScene::CreateResource()
    {
		particles = std::make_unique<Regal::Graphics::Particles>(
			Regal::Graphics::Graphics::Instance().GetDevice(), 1000);
    }

    void DemoScene::Initialize()
    {
		particles->Initialize(Regal::Graphics::Graphics::Instance().GetDeviceContext(), 0);
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

		//2D
		{
			graphics.Set2DStates();
		}

		//パーティクル
		graphics.SetStates(Graphics::Graphics::ZT_ON_ZW_ON, Graphics::Graphics::CULL_NONE, Graphics::Graphics::ALPHA);
		immediateContext->GSSetConstantBuffers(1, 1, graphics.GetShader()->GetSceneConstanceBuffer().GetAddressOf());
		particles->Render(immediateContext);

		//3D
		{
			graphics.Set3DStates();

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
    }

    void DemoScene::DrawDebug()
    {
		particles->DrawDebug();
    }
}