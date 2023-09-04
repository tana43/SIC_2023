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

        static bool isDebugRender;

    private:
        std::unique_ptr<Regal::Resource::GltfModel> gltfModel;

        std::unique_ptr<Regal::Graphics::Particles> particles;
    };
}
