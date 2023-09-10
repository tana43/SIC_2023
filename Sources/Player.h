#pragma once
#include "RegalLib/Regal.h"
#include "BlockGroup.h"

class Player : public Regal::Game::GameObject
{
public:
    Player() : GameObject("Player") {}
    ~Player() {}

    void CreateResource()override {}
    void Initialize()override;
    void Update(float elapsedTime)override;
    void Render()override {}
    void DrawDebug()override;

    void UseBlocksMove();

    void AutoFallBlock(float elapsedTime);
    void AutoSetBlock(float elapsedTime);

    //�g�p�u���b�N��؂�ւ���Ƃ��ɌĂ�
    void ChangeUseBG();

    //----------------------------INPUT-------------------------------------
    
    //����{�^��
    static const bool SelectButton()
    {
        return Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::Enter);
    }

    //���E�㉺�ړ�
    static const bool MoveUpButton()
    {
        return Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::W);
    }
    static const bool MoveLeftButton()
    {
        return Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::A);
    }
    static const bool MoveDownButton()
    {
        return Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::S);
    }
    static const bool MoveRightButton()
    {
        return Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::D);
    }

    //�E���A�����ړ�
    static const bool MoveBottomRightButton()
    {
        return Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::C);
    }
    static const bool MoveBottomLeftButton()
    {
        return Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::Z);
    }

    //���E��]
    static const bool RotRightButton()
    {
        return Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::E);
    }
    static const bool RotLeftButton()
    {
        return Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::Q);
    }

    //�X�y�[�X�ŔՖʃN���A������悩
    static const bool AllBlockClearButton()
    {
        return Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::Space);
    }

    //----------------------------------------------------------------------

    void SetUseBlockGroup(BlockGroup* blockGroup) 
    { 
        useBlockGroup = blockGroup;
        useBlockGroup->SetOnGrid(true);
    }
    BlockGroup* GetUseBlockGroup() { return useBlockGroup; }

private:
    BlockGroup* useBlockGroup;

    int hp;
    int power;//�U����

    //�u���b�N�̎�������
    float autoFallTime;
    float autoFallTimer;

    //�u���b�N�����ݒu����
    float autoSetTime;
    float autoSetTimer;
};

