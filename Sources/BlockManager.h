#pragma once
#include "RegalLib/Game/GameObject.h"
#include <vector>

#include "Block.h"
#include "PuzzleFrame.h"

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

    //生成したブロックの登録
    void Register(Block* block);

    void Remove(Block* block);

    std::vector<Block*>& GetBlocks() { return blocks; }

    //引数と同じアビリティを持ったブロックの配列を作成し、返す
    std::vector<Block> GetSameAbilityBlocks(PuzzleFrame::ChainAbility* cAbility);

private:
    std::vector<Block*> blocks;
    std::vector<Block*> removes;
};

