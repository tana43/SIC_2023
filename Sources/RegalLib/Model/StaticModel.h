#pragma once
#include "Model.h"

namespace Regal::Model
{
    class StaticModel : public Regal::Model::Model
    {
    public:
        StaticModel(const char* fbxFilename, bool traiangulate = false);
        ~StaticModel() {}

        void Render();

        void DrawDebug();

        Regal::Game::Transform& GetTransform() { return transform; }

        void SetEmissiveColor(const DirectX::XMFLOAT4& col) { emissiveColor = col; }
        const DirectX::XMFLOAT4 GetEmissiveColor() const { return emissiveColor; }


        void SetEmissiveIntensity(const float& intensity) { emissiveIntencity = intensity; }
        const float GetEmissiveIntensity() const { return emissiveIntencity; }

    private:
        DirectX::XMFLOAT4 emissiveColor = {1.0f,1.0f,1.0f,1.0f};
        float emissiveIntencity = 3.0f;
    };

}
