#pragma once
#include "RegalLib/Game/GameObject.h"
#include <vector>

#include "BlockGroup.h"

class BlockGroupManager
{
private:
    BlockGroupManager() {}
    ~BlockGroupManager();

public:
    static BlockGroupManager& Instance()
    {
        static BlockGroupManager instance;
        return instance;
    }

    void Initialize();
    void Update(float elapsedTime);
    void Render();
    void DrawDebug();

    void Clear();

    //生成したブロックの登録
    void Register(BlockGroup* BlockGroup);

    void Remove(BlockGroup* BlockGroup);

    std::vector<BlockGroup*>& GetBlockGroups() { return blockGroups; }

private:
    std::vector<BlockGroup*> blockGroups;
    std::vector<BlockGroup*> removes;
};

