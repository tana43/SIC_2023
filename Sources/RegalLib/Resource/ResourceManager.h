#pragma once
#include <map>
#include <string>
#include <memory>

#include "../Resource/SkinnedMesh.h"

class ResourceManager
{
private:
    ResourceManager() {}
    ~ResourceManager() {}

public:
    static ResourceManager& Instance()
    {
        static ResourceManager instance;
        return instance;
    }

    //モデルリソース読み込み
    const std::shared_ptr<Regal::Resource::SkinnedMesh> LoadModelResource(
        ID3D11Device* device,
        const char* const fbxFilename,
        const bool triangulate = false,
        const float samplingRate = 0
    );

private:
    using ModelMap = std::map<const char*, std::weak_ptr<Regal::Resource::SkinnedMesh>>;
 
private:
    ModelMap models = {};
};

