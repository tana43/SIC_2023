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
    else if (MoveBottomRightButton())
    {
        if (useBlockGroup->MoveBottomRight(1))
        {
            //�����������ԃ��Z�b�g
            autoFallTimer = 0;

            //�����ݒu���ԃ��Z�b�g
            autoSetTimer = 0;
        }
    }
    else if (MoveBottomLeftButton())
    {
        if (useBlockGroup->MoveBottomLeft(1))
        {
            //�����������ԃ��Z�b�g
            autoFallTimer = 0;

            //�����ݒu���ԃ��Z�b�g
            autoSetTimer = 0;
        }
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

void Player::AutoSetBlock(float elapsedTime)
{
    //�ڒn���Ă���
    if (useBlockGroup->IsBottom() && !useBlockGroup->CanMoveDown())
    {
        autoSetTimer += elapsedTime;
        autoFallTimer = 0;
    }
    else//�ڒn���Ă��Ȃ�
    {
        autoSetTimer = 0;
    }

    //�u���b�N��ݒu
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
