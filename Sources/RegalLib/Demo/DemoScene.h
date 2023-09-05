#pragma once

#include "../Regal.h"

namespace Regal::Demo
{
    class DemoScene final : public Regal::Scene::BaseScene
    {
    public:
        DemoScene() {}
        ~DemoScene()override {}

        void CreateResource()override;
        void Initialize() override;
        void Finalize() override;
        void Begin() override;
        void Update(const float& elapsedTime)override;
        void End() override;
        void Render(const float& elapsedTime)override;

        void DrawDebug()override;
        void PostEffectDrawDebug()override;

        static bool isDebugRender;

    private:
        float clearColor[4] = { 0.1f,0.1f,0.1f,1.0f };

        std::unique_ptr<Regal::Resource::GltfModel> gltfModel;
        std::unique_ptr<Regal::Graphics::Particles> particles;
        std::unique_ptr<Regal::Graphics::Framebuffer> framebuffer;
        std::unique_ptr<Regal::Graphics::Bloom> bloomer;
        std::unique_ptr<Regal::Graphics::FullscreenQuad> bitBlockTransfer;

        //輝度成分抽出用シェーダー
        Microsoft::WRL::ComPtr<ID3D11PixelShader> LEPixelShader;

        std::unique_ptr<Regal::Model::StaticModel> model;

        std::unique_ptr<Regal::Resource::Sprite> sprite;
    };
}
