#pragma once
#include "Block.h"
#include "RegalLib/Regal.h"

class BlocksGroup : Regal::Game::GameObject
{
public:
    BlocksGroup();
    ~BlocksGroup() {}

    void CreateResource()override;
    void Initialize()override;
    void Update(float elapsedTime)override;
    void Render()override;
    void DrawDebug()override;

private:
    Block* blocks[4];
};

