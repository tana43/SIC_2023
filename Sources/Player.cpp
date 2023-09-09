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
    //操作対象のブロックがなければ処理しない
    if (!useBlockGroup)return;

    //上方向以外への移動
    if (MoveDownButton())
    {
        if (useBlockGroup->MoveDown(1))
        {
            //自動落下時間リセット
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

    //ブロック回転
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
