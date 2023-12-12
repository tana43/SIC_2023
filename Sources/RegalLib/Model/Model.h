#pragma once
#include "../Game/Transform.h"
#include "../Resource/SkinnedMesh.h"

namespace Regal::Model
{
    class Model
    {
    public:
        Model() {}
        virtual ~Model() {}

        virtual void Render() = 0;

        virtual void DrawDebug() = 0;

        Regal::Resource::SkinnedMesh* GetSkinnedMesh() { return skinnedMesh.get(); }

    protected:
        Regal::Game::TransformEuler transform;

        std::shared_ptr<Regal::Resource::SkinnedMesh> skinnedMesh;
    };
}
