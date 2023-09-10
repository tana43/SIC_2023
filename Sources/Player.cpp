#include "Player.h"
#include "PuzzleFrame.h"
#include "GameManager.h"

void Player::Initialize()
{
    hp = 10;
    power = 0;
    autoFallTime = 1.5f;
    autoFallTimer = 0;
    autoSetTime = 0.5f;
    autoSetTimer = 0;
}

void Player::Update(float elapsedTime)
{
    //useBlockGroup->Update(elapsedTime);

    UseBlocksMove();

    AutoFallBlock(elapsedTime);

    AutoSetBlock(elapsedTime);
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
    else if (MoveBottomRightButton())
    {
        if (useBlockGroup->MoveBottomRight(1))
        {
            //自動落下時間リセット
            autoFallTimer = 0;

            //自動設置時間リセット
            autoSetTimer = 0;
        }
    }
    else if (MoveBottomLeftButton())
    {
        if (useBlockGroup->MoveBottomLeft(1))
        {
            //自動落下時間リセット
            autoFallTimer = 0;

            //自動設置時間リセット
            autoSetTimer = 0;
        }
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

void Player::AutoSetBlock(float elapsedTime)
{
    //接地している
    if (useBlockGroup->IsBottom() && !useBlockGroup->CanMoveDown())
    {
        autoSetTimer += elapsedTime;
        autoFallTimer = 0;
    }
    else//接地していない
    {
        autoSetTimer = 0;
    }

    //ブロックを設置
    if (autoSetTimer > autoSetTime)
    {
        useBlockGroup->PutOnGrid();
        autoSetTimer = 0;

        ChangeUseBG();
    }
}

void Player::ChangeUseBG()
{
    GameManager::Instance().NextBlockUse();


}
