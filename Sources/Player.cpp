#include "Player.h"
#include "PuzzleFrame.h"
#include "GameManager.h"

void Player::CreateResource()
{
    model = std::make_unique<Regal::Model::StaticModel>("./Resources/Models/LuminousCube04.fbx");
}

void Player::Initialize()
{
    hp = maxHp;
    power = 0;
    autoFallTime = 1.5f;
    autoFallTimer = 0;
    autoSetTime = 0.5f;
    autoSetTimer = 0;

    model->GetTransform()->SetPositionX(-45.0f);
    model->GetTransform()->SetPositionY(72.0f);
    model->GetTransform()->SetScaleFactor(2.3f);
}

void Player::Update(float elapsedTime)
{
    //useBlockGroup->Update(elapsedTime);

    model->GetTransform()->AddRotationY(-0.1f * elapsedTime);
    model->GetTransform()->AddRotationX(0.1f * elapsedTime);

    UseBlocksMove();

    AutoFallBlock(elapsedTime);

    AutoSetBlock(elapsedTime);

    //hp�ɘA�����ē_�ł�����
    if (hp > 0)
    {
        float intensity{ sinf(blinkTimer) * (static_cast<float>(hp) / static_cast<float>(maxHp)) * 2.5f };
        if (intensity < 0)intensity = -intensity;
        model->GetSkinnedMesh()->SetEmissiveIntensity(intensity);
        blinkTimer += elapsedTime;
    }
    else
    {
        model->GetSkinnedMesh()->SetEmissiveIntensity(0);
    }
}

void Player::Render()
{
    model->Render();
}

void Player::DrawDebug()
{
    ImGui::Begin(name.c_str());

    model->DrawDebug();
    
    if (useBlockGroup)
    {
        useBlockGroup->DrawDebug();
    }

    ImGui::SliderInt("HP", &hp, 0, maxHp);

    ImGui::End();
    
}

void Player::UseBlocksMove()
{
    //����Ώۂ̃u���b�N���Ȃ���Ώ������Ȃ�
    if (!useBlockGroup)return;

    //����͂Œ�ɍő��u���b�N�z�u
    if (MoveUpButton())
    {
        while (useBlockGroup->MoveDown(1));
        autoSetTimer = autoSetTime;
    }

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
        auto* bg{ useBlockGroup };
        autoSetTimer = 0;

        ChangeUseBG();

        //�u���b�N���������C���ɒB���Ă���Ȃ�Ֆʂ�S�ă��Z�b�g
        bg->OutFrame();
    }
}

void Player::ChangeUseBG()
{
    GameManager::Instance().NextBlockUse();
}

bool Player::ApplyDamage(int damage)
{
    //�_���[�W���O�̏ꍇ�͌��N��Ԃ�ύX����K�v���Ȃ�
    if (damage == 0)return false;

    //���S���Ă���ꍇ�͌��N��Ԃ�ύX���Ȃ�
    if (hp <= 0)return false;

    hp -= damage;

    if (hp <= 0)
    {
        OnDead();
    }
    else
    {
        OnDamaged();
    }

    return true;
}

void Player::OnDamaged()
{
    Regal::Game::Camera::Instance().ScreenVibrate(0.08f,0.7f);
}

void Player::OnDead()
{
}
