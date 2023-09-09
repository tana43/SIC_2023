#include "Player.h"

void Player::Initialize()
{
    hp = 10;
    power = 0;
    autoFallTime = 1.5f;
    autoFallTimer = 0;
}

void Player::Update(float elapsedTime)
{
    UseBlocksMove();

    AutoFallBlock(elapsedTime);
}

void Player::DrawDebug()
{
    ImGui::Begin(name.c_str());
    
    if (useBlockGroup)
    {
        useBlockGroup->DrawDebug();
    }

    ImGui::End();
    
}

void Player::UseBlocksMove()
{
    //����Ώۂ̃u���b�N���Ȃ���Ώ������Ȃ�
    if (!useBlockGroup)return;

    //������ȊO�ւ̈ړ�
    if (MoveDownButton())
    {
        if (useBlockGroup->MoveDown(1))
        {
            //�����������ԃ��Z�b�g
            autoFallTimer = 0;
        }
    }
    else if (MoveRightButton())
    {
        useBlockGroup->MoveRight(1);
    }
    else if (MoveLeftButton())
    {
        useBlockGroup->MoveLeft(1);
    }

    //�u���b�N��]
    if (RotRightButton())
    {
        useBlockGroup->RotRight();
    }
    else if (RotLeftButton())
    {
        useBlockGroup->RotLeft();
    }
}

void Player::AutoFallBlock(float elapsedTime)
{
    if (autoFallTimer > autoFallTime)
    {
        useBlockGroup->MoveDown(1);
        autoFallTimer = 0;
    }

    autoFallTimer += elapsedTime;
}
