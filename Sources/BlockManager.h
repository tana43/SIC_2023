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

    //���������u���b�N�̓o�^
    void Register(Block* block);

    void Remove(Block* block);

    std::vector<Block*>& GetBlocks() { return blocks; }

    //�����Ɠ����A�r���e�B���������u���b�N�̔z����쐬���A�Ԃ�
    std::vector<Block> GetSameAbilityBlocks(PuzzleFrame::ChainAbility* cAbility);

private:
    std::vector<Block*> blocks;
    std::vector<Block*> removes;
};

