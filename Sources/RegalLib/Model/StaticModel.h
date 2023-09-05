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

        Regal::Game::Transform* GetTransform() { return &transform; }
    };

}
