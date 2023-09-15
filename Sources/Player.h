#pragma once
#include "RegalLib/Regal.h"
#include "BlockGroup.h"

class Player : public Regal::Game::GameObject
{
public:
    //�v���C���[��p�̒e��
    struct Projectile
    {
        void CreateResource();
        void Initialize();
        void Update(float elapsedTime);
        void Render();
        void DrawDebug();

        void Hit();

        void Remove();

        float speed{40.0f};
        int power;
        int type;
        float chargeTimer;
        float chargeTime{5.0f};
        float spinSpeed{0.2f};
        bool completeCharge{false};

        float scale{0};

        std::unique_ptr<Regal::Model::StaticModel> model;
        
        Player* owner;
    };

    Player() : GameObject("Player") {}
    ~Player() {}

    void CreateResource()override;
    void Initialize()override;
    void Update(float elapsedTime)override;
    void Render()override;
    void DrawDebug()override;

    void UseBlocksMove();

    void AutoFallBlock(float elapsedTime);
    void AutoSetBlock(float elapsedTime);

    //�g�p�u���b�N��؂�ւ���Ƃ��ɌĂ�
    void ChangeUseBG();

    bool ApplyDamage(int damage);

    void OnDamaged();
    void OnDead();

    void Heal(int value);

    Regal::Game::Transform* GetTransform() { return model->GetTransform(); }

    void SetColor(const DirectX::XMFLOAT4 color) { model->GetSkinnedMesh()->SetEmissiveColor(color); }

    void Shot(int type);

    void ProjectilesClear();



    //----------------------------INPUT-------------------------------------
    
    //����{�^��
    static const bool SelectButton()
    {
        return (Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::Enter) ||
            (GamePad::BTN_A & Regal::Input::Input::GetGamePad().GetButtonDown()));
    }

    //���E�㉺�ړ�
    static const bool MoveUpButton()
    {
        return (Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::W) ||
            (GamePad::BTN_UP & Regal::Input::Input::GetGamePad().GetButtonDown()));
    }
    static const bool MoveLeftButton()
    {
        return (Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::A) ||
            (GamePad::BTN_LEFT & Regal::Input::Input::GetGamePad().GetButtonDown()));
    }
    static const bool MoveDownButton()
    {
        return (Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::S) ||
            (GamePad::BTN_DOWN & Regal::Input::Input::GetGamePad().GetButtonDown()));
    }
    static const bool MoveRightButton()
    {
        return (Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::D) ||
            (GamePad::BTN_RIGHT & Regal::Input::Input::GetGamePad().GetButtonDown()));
    }

    //�E���A�����ړ�
    static const bool MoveBottomRightButton()
    {
        return ((Regal::Input::Keyboard::GetKeyState().LeftShift && MoveRightButton() )||
            ((GamePad::BTN_RIGHT_SHOULDER & Regal::Input::Input::GetGamePad().GetButtonDown())));
    }
    static const bool MoveBottomLeftButton()
    {
        return ((Regal::Input::Keyboard::GetKeyState().LeftShift && MoveLeftButton()) ||
            ((GamePad::BTN_LEFT_SHOULDER & Regal::Input::Input::GetGamePad().GetButtonDown())));
    }

    //���E��]
    static const bool RotRightButton()
    {
        return (Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::E) ||
            Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::Space) ||
            (GamePad::BTN_B & Regal::Input::Input::GetGamePad().GetButtonDown()));
    }
    static const bool RotLeftButton()
    {
        return (Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::Q) ||
            (GamePad::BTN_A & Regal::Input::Input::GetGamePad().GetButtonDown()));
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

    Regal::Model::StaticModel* GetModel() { return model.get(); }

    PopEffect* GetProjectilePopEffect(int type) { return projectilePopEffects[type].get(); }

    void SetPower(const int power,const char t) { this->power[t] = power; }
    const int GetPower(const char type) const { return power[type]; }

    std::vector<Projectile*>& GetRemoves() { return removes; }

private:
    BlockGroup* useBlockGroup;

    //�������U�����������Ƃ��̓J������U��
    //�G�ɍU����^�����Ƃ��͓G���̂�U��������

    int hp;
    int maxHp{100};
    int power[Block::END];//�e�����̍U����

    float attackTimer[Block::END];
    float attackTime{6.0f};

    //�u���b�N�̎�������
    float autoFallTime;
    float autoFallTimer;

    //�u���b�N�����ݒu����
    float autoSetTime;
    float autoSetTimer;

    std::unique_ptr<Regal::Model::StaticModel> model;

    float blinkTimer;
    float blinkIntensity;

    std::vector<Projectile*> projectiles;
    std::vector<Projectile*> removes;

    std::unique_ptr<PopEffect> projectilePopEffects[Block::BlockType::END];

    std::unique_ptr<Regal::Resource::Sprite> hpGauge;
    DirectX::XMFLOAT2 spritePos{34,670};
};

