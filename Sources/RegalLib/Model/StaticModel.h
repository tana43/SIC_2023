#pragma once
#include "Model.h"

namespace Regal::Model
{
    class StaticModel : public Regal::Model::Model
    {
    public:
        StaticModel(ID3D11Device* device, const char* fbxFilename, bool traiangulate = true);
        ~StaticModel() {}

        void Render();

        void DrawDebug();

        Regal::Game::Transform* GetTransform() { return &transform; }
    };

}
