#include "ResourceManager.h"

const std::shared_ptr<Regal::Resource::SkinnedMesh> ResourceManager::LoadModelResource(
    ID3D11Device* device, 
    const char* const fbxFilename, 
    const bool triangulate, 
    const float samplingRate)
{
    //モデル検索
    const ModelMap::iterator it = models.find(fbxFilename);
    if (it != models.end())
    {
        //リンク(寿命)が切れていないか確認
        if (it->second.expired() == false)
        {
            //検索したモデルと同じモデルが見つかったらそれを返す
            return it->second.lock();
        }
    }

    //見つからなかった新規モデルリソースを作成・読み込み
    const std::shared_ptr<Regal::Resource::SkinnedMesh> model = {
        std::make_shared<Regal::Resource::SkinnedMesh>(
            device,
            fbxFilename,
            triangulate,
            samplingRate)
    };

    //マップに登録
    models[fbxFilename] = model;

    return model;
}
