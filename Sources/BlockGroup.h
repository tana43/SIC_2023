#pragma once
#include "Block.h"
#include "RegalLib/Regal.h"

class BlockGroup : Regal::Game::GameObject
{
public:
    //生成グリッド座標
    static const int GenerationPosX = 14;
    static const int GenerationPosY = 14;

    BlockGroup(bool onGrid = true);
    ~BlockGroup() {}

    void CreateResource()override;
    void Initialize()override;
    void Update(float elapsedTime)override;
    void Render()override;
    void DrawDebug()override;

    bool MoveDown(int moveDistance);
    void MoveRight(int moveDistance);
    void MoveLeft(int moveDistance);

    void RotRight();
    void RotLeft();

    DirectX::XMFLOAT3 GetPosition() const { return position; }
    void SetPosition(const DirectX::XMFLOAT3 pos) { position = pos; }

    void SetOnGrid(const bool og)
    {
        onGrid = og;
        for (auto& block : blocks)
        {
            block->SetOnGrid(og);
        }
    }

private:
    Block* blocks[4];

    Block::GridPosition gridPos{GenerationPosX,GenerationPosY};

    //onGridがfalseの場合に使う位置情報
    DirectX::XMFLOAT3 position;

    bool onGrid;
};

