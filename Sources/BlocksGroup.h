#pragma once
#include "Block.h"
#include "RegalLib/Regal.h"

class BlocksGroup : Regal::Game::GameObject
{
public:
    //生成グリッド座標
    static const int GenerationPosX = 12;
    static const int GenerationPosY = 10;

    BlocksGroup();
    ~BlocksGroup() {}

    void CreateResource()override;
    void Initialize()override;
    void Update(float elapsedTime)override;
    void Render()override;
    void DrawDebug()override;

private:
    Block* blocks[4];

    Block::GridPosition gridPos{GenerationPosX,GenerationPosY};
};

