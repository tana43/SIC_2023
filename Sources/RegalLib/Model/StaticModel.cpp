#include "StaticModel.h"
#include "../Graphics/Graphics.h"

namespace Regal::Model
{
    StaticModel::StaticModel(ID3D11Device* device, const char* fbxFilename, bool traiangulate)
    {
        skinnedMesh = std::make_unique<Regal::Resource::SkinnedMesh>(device,fbxFilename,traiangulate);
    }

    void StaticModel::Render()
    {
        skinnedMesh->_Render(Regal::Graphics::Graphics::Instance().GetDeviceContext(),&transform);
    }

    void StaticModel::DrawDebug()
    {
        transform.DrawDebug();
    }
}