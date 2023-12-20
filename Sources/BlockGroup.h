#pragma once
#include "Block.h"
#include "RegalLib/Regal.h"

class BlockGroup : Regal::Game::GameObject
{
public:
    //�����O���b�h���W
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
    bool MoveBottomRight(int moveDistance);
    bool MoveBottomLeft(int moveDistance);

    void RotRight();
    void RotLeft();

    DirectX::XMFLOAT3 GetPosition() const { return position; }
    void SetPosition(const DirectX::XMFLOAT3 pos) { position = pos; }

    const Block::GridPosition& GetGridPosition() const { return gridPos; }
    void SetGridPosition(const Block::GridPosition gridPosition) { gridPos = gridPosition; }

    void SetOnGrid(const bool og)
    {
        onGrid = og;
        for (auto& block : blocks)
        {
            block->SetOnGrid(og);
        }
    }

    //�ݒu���ꂽ�u���b�N����ɂ��Ă��邩
    bool IsBottom();

    //�u���b�N��g���ɐݒu�A�Œ�
    void PutOnGrid();

    const bool CanMoveDown();

    //��̐������C�����z�����Ƃ��̏���
    void OutFrame();

    Block& GetBlocks(int index) { return *blocks[index]; }

    //���݂̃u���b�N�O���[�v�̍��W���S�̊e�u���b�N�ɔ��f������
    void ReflectionGridPosition();

private:
    Block* blocks[4];

    Block::GridPosition gridPos{GenerationPosX,GenerationPosY};

    //onGrid��false�̏ꍇ�Ɏg���ʒu���
    DirectX::XMFLOAT3 position;

    bool onGrid;


};

