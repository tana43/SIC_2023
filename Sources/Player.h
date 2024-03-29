#pragma once
#include "RegalLib/Regal.h"
#include "BlockGroup.h"

class Player : public Regal::Game::GameObject
{
public:
    //プレイヤー専用の弾丸
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

    void UseBlocksMove(float elapsedTime);

    void AutoFallBlock(float elapsedTime);
    void AutoSetBlock(float elapsedTime);

    //使用ブロックを切り替えるときに呼ぶ
    void ChangeUseBG();

    bool ApplyDamage(int damage);
    void ApplyHeal(int value);

    void OnDamaged();//ダメージを受けたときに呼ばれる
    void OnDead();//死んだときに呼ばれる
    void OnHealed();//回復された時に呼ばれる

    void GuideUpdate();


    Regal::Game::Transform& GetTransform() { return model->GetTransform(); }

    void SetColor(const DirectX::XMFLOAT4 color) { model->SetEmissiveColor(color); }

    void Shot(int type);

    void ProjectilesClear();



    //----------------------------INPUT-------------------------------------
    

    //決定ボタン
    static const bool SelectButton()
    {
        return (Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::Enter) ||
            (GamePad::BTN_A & Regal::Input::Input::GetGamePad().GetButtonDown()));
    }
    //キャンセルボタン
    static const bool BackButton()
    {
        return (Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::Escape) ||
            (GamePad::BTN_B & Regal::Input::Input::GetGamePad().GetButtonDown()));
    }

    //左右上下移動
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

    //左右上下高速移動
    static const bool FastMoveLeftButton(float elapsedTime)
    {
        if (Regal::Input::Keyboard::GetKeyState().A ||
            (GamePad::BTN_LEFT & Regal::Input::Input::GetGamePad().GetButton()))
        {
            inputTimer[0] += elapsedTime;
        }
        else
        {
            inputTimer[0] = 0;
        }

        if (inputTimer[0] > longPressTime)
        {
            inputTimer[0] -= longPressInter;
            return true;
        }
        else
        {
            return false;
        }
    }
    static const bool FastMoveDownButton(float elapsedTime)
    {
        if (Regal::Input::Keyboard::GetKeyState().S ||
            (GamePad::BTN_DOWN & Regal::Input::Input::GetGamePad().GetButton()))
        {
            inputTimer[1] += elapsedTime;
        }
        else
        {
            inputTimer[1] = 0;
        }

        if (inputTimer[1] > longPressTime)
        {
            inputTimer[1] -= longPressInter;
            return true;
        }
        else
        {
            return false;
        }
    }
    static const bool FastMoveRightButton(float elapsedTime)
    {
        if (Regal::Input::Keyboard::GetKeyState().D ||
            (GamePad::BTN_RIGHT & Regal::Input::Input::GetGamePad().GetButton()))
        {
            inputTimer[2] += elapsedTime;
        }
        else
        {
            inputTimer[2] = 0;
        }

        if (inputTimer[2] > longPressTime)
        {
            inputTimer[2] -= longPressInter;
            return true;
        }
        else
        {
            return false;
        }
    }

    //右下、左下移動
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
    static const bool FastMoveBottomRightButton(float elapsedTime)
    {
        if ((Regal::Input::Keyboard::GetKeyState().LeftShift && Regal::Input::Keyboard::GetKeyState().D) ||
            ((GamePad::BTN_RIGHT_SHOULDER & Regal::Input::Input::GetGamePad().GetButton())))
        {
            inputTimer[3] += elapsedTime;
        }
        else
        {
            inputTimer[3] = 0;
        }

        if (inputTimer[3] > longPressTime)
        {
            inputTimer[3] -= longPressInter;
            return true;
        }
        else
        {
            return false;
        }
    }
    static const bool FastMoveBottomLeftButton(float elapsedTime)
    {
        if ((Regal::Input::Keyboard::GetKeyState().LeftShift && Regal::Input::Keyboard::GetKeyState().A) ||
            ((GamePad::BTN_LEFT_SHOULDER & Regal::Input::Input::GetGamePad().GetButton())))
        {
            inputTimer[4] += elapsedTime;
        }
        else
        {
            inputTimer[4] = 0;
        }

        if (inputTimer[4] > longPressTime)
        {
            inputTimer[4] -= longPressInter;
            return true;
        }
        else
        {
            return false;
        }
    }

    //左右回転
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

    //スペースで盤面クリアもいれよか
    static const bool AllBlockClearButton()
    {
        return Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::Space);
    }

    static const bool PauseButton()
    {
        return Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::Escape) ||
            (GamePad::BTN_START & Regal::Input::Input::GetGamePad().GetButtonDown());
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

    static float inputTimer[5];
    static constexpr float longPressTime = 0.3f;
    static constexpr float longPressInter = 0.05f;

    BlockGroup* useBlockGroup;

    //自分が攻撃を喰らったときはカメラを振動
    //敵に攻撃を与えたときは敵自体を振動させる

    int hp;
    int maxHp{100};
    int power[Block::END];//各属性の攻撃力

    float attackTimer[Block::END];
    float attackTime{6.0f};

    //ブロックの自動落下
    float autoFallTime;
    float autoFallTimer;

    //ブロック強制設置時間
    float autoSetTime;
    float autoSetTimer;

    std::unique_ptr<Regal::Model::StaticModel> model;

    float blinkTimer;
    float blinkIntensity;

    std::vector<Projectile*> projectiles;
    std::vector<Projectile*> removes;

    std::unique_ptr<PopEffect> projectilePopEffects[Block::BlockType::END];

    std::unique_ptr<Regal::Resource::Sprite> hpGauge;
    std::unique_ptr<Regal::Resource::Sprite> hpHealGauge;//回復されたときに先に回復分の合計量を分かりやすくするゲージ

    float healedTimer = 10;//回復されてから何秒経過したかのタイマー(０にすると最初の数秒,緑のゲージが出てしまうので適当な値を入れとく)

    float hpGaugeDamageTimer = 1;

    std::unique_ptr<BlockGroup> guideBlock;
};

