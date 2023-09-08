#pragma once
#include "RegalLib/Game/GameObject.h"
#include <vector>

#include "Block.h"

class BlockManager
{
private:
    BlockManager() {}
    ~BlockManager();

public:
    static BlockManager& Instance()
    {
        static BlockManager instance;
        return instance;
    }

    void Initialize();
    void Update(float elapsedTime);
    void Render();
    void DrawDebug();

    void Clear();

    //ê∂ê¨ÇµÇΩÉuÉçÉbÉNÇÃìoò^
    void Register(Block* block);

    void Remove(Block* block);

    std::vector<Block*>& GetBlocks() { return blocks; }

private:
    std::vector<Block*> blocks;
    std::vector<Block*> removes;
};

