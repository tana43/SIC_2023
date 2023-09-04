#include "StaticModel.h"

namespace Regal::Model
{
    StaticModel::StaticModel(ID3D11Device* device, const char* fbxFilename, bool traiangulate)
    {
        skinnedMesh = std::make_unique<Regal::Resource::SkinnedMesh>(device,fbxFilename,traiangulate);
    }
    void StaticModel::Render()
    {
    }
    void StaticModel::DrawDebug()
    {
    }
}