#include "Player.h"
#include "PuzzleFrame.h"
#include "GameManager.h"
#include "EnemyManager.h"
#include "AudioManager.h"

float Player::inputTimer[] = {0,0,0,0,0};

void Player::CreateResource()
{
    model = std::make_unique<Regal::Model::StaticModel>("./Resources/Models/LuminousCube04.fbx");
    for (auto& effect : projectilePopEffects)
    {
        effect = std::make_unique<PopEffect>(10);
    }

    hpGauge = std::make_unique<Regal::Resource::Sprite>(Regal::Graphics::Graphics::Instance().GetDevice(),
        L"./Resources/Images/PlayerHpGauge.png","HPGauge");
    hpHealGauge = std::make_unique<Regal::Resource::Sprite>(Regal::Graphics::Graphics::Instance().GetDevice(),
        L"./Resources/Images/PlayerHpGauge.png","HPHealGauge");
}

void Player::Initialize()
{
    hp = maxHp;
    for (int i = 0; i < Block::END; i++)
    {
        power[i] = 0;
    }
    autoFallTime = 1.5f;
    autoFallTimer = 0;
    autoSetTime = 0.5f;
    autoSetTimer = 0;

    model->GetTransform().SetPositionX(-45.0f);
    model->GetTransform().SetPositionY(72.0f);
    model->GetTransform().SetScaleFactor(2.3f);

    inputTimer[0] = 0.0f;
    inputTimer[1] = 0.0f;
    inputTimer[2] = 0.0f;
    inputTimer[3] = 0.0f;
    inputTimer[4] = 0.0f;


    for (int i = 0;i < Block::END;++i)
    {
        switch (i)
        {
        case Block::RED:
            projectilePopEffects[i]->SetColor(DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
            break;
        case Block::CYAN:
            projectilePopEffects[i]->SetColor(DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f));
            break;
        case Block::GREEN:
            projectilePopEffects[i]->SetColor(DirectX::XMFLOAT4(0.4f, 1.0f, 0.0f, 1.0f));
            break;
        case Block::PURPLE:
            projectilePopEffects[i]->SetColor(DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f));
            break;
        }
    }
    
    hpGauge->GetSpriteTransform().SetPosition(DirectX::XMFLOAT2(265,250));
    hpGauge->GetSpriteTransform().SetScaleY(1.9f);

    hpHealGauge->GetSpriteTransform().SetPosition(DirectX::XMFLOAT2(265, 250));
    hpHealGauge->GetSpriteTransform().SetScaleY(1.9f);
    hpHealGauge->SetColor(DirectX::XMFLOAT4(0, 1, 0, 1));

    //落下地点を示したガイド表示用のブロック
    guideBlock = std::make_unique<BlockGroup>();
    guideBlock->CreateResource();
    guideBlock->Initialize();
}

void Player::Update(float elapsedTime)
{
    //useBlockGroup->Update(elapsedTime);
    

    model->GetTransform().AddRotationY(-0.1f * elapsedTime);
    model->GetTransform().AddRotationX(0.1f * elapsedTime);

    //全攻撃中は処理を止める
    if (!PuzzleFrame::Instance().GetIsFrameAttack())
    {
        UseBlocksMove(elapsedTime);

        AutoFallBlock(elapsedTime);

        AutoSetBlock(elapsedTime);
    }

    //hpに連動して点滅させる
    if (hp > 0)
    {
        float intensity{ sinf(blinkTimer) * (static_cast<float>(hp) / static_cast<float>(maxHp)) * 2.5f };
        if (intensity < 0)intensity = -intensity;
        model->SetEmissiveIntensity(intensity);
        blinkTimer += elapsedTime;
    }
    else
    {
        model->SetEmissiveIntensity(0);
    }

    //クールダウンと攻撃
    for (int i = 0; i < Block::END; i++)
    {
        if (power[i] <= 0)continue;

        if (attackTimer[i] > attackTime)
        {
            Shot(i);
            attackTimer[i] = 0;
        }

        attackTimer[i] += elapsedTime;
    }

    for (auto& projectile : projectiles)
    {
        projectile->Update(elapsedTime);
    }

    for (auto& effect : projectilePopEffects)
    {
        effect->Update(elapsedTime);
    }


    for (auto& projectile : removes)
    {
        //イテレーターからじゃないと破棄できない
        std::vector<Projectile*>::iterator it =
            std::find(projectiles.begin(), projectiles.end(), projectile);

        if (it != projectiles.end())
        {
            projectiles.erase(it);
        }

        delete projectile;
    }
    removes.clear();

    //ダメージを喰らった際に赤くなったゲージが徐々に白に戻る
    hpGauge->FadeColor(DirectX::XMFLOAT4(1, 1, 1, hpGauge->GetAlpha()), hpGaugeDamageTimer, 1);
    hpGaugeDamageTimer += elapsedTime;

    //自機が瀕死状態のときにHPゲージの点滅
    if (hp < maxHp / 4)
    {
        static bool isFadeIn = false;
        if (isFadeIn)
        {
            if (hpGauge->FadeIn(1.0f, elapsedTime * 3))isFadeIn = false;
        }
        else
        {
            if (hpGauge->FadeOut(0.3f, elapsedTime * 3))isFadeIn = true;
        }
    }
    else
    {
        //通常の表示
        hpGauge->FadeIn(1.0f, elapsedTime * 3);
    }

    //回復後タイマー更新
    healedTimer += elapsedTime;

    //回復タイマーが2秒以上経過していなければHPゲージの更新を止め,
    //2~3秒の間なら回復ゲージに追いつくように本体のゲージを伸ばしていく
    if (healedTimer > 4)
    {
        hpGauge->GetSpriteTransform().SetScaleX(static_cast<float>(hp) / static_cast<float>(maxHp));
        //if(hpHealGauge->GetSpriteTransform().GetScaleX())
    }
    else if(healedTimer < 2)
    {
        hpHealGauge->GetSpriteTransform().SetScaleX(static_cast<float>(hp) / static_cast<float>(maxHp));
    }
    else
    {
        //2~4秒の間
        const float alpha = (healedTimer - 2) / 2;
        hpGauge->GetSpriteTransform().SetScaleX(
            hpGauge->GetSpriteTransform().GetScaleX() +
            (hpHealGauge->GetSpriteTransform().GetScaleX() - 
                hpGauge->GetSpriteTransform().GetScaleX()) * alpha);
    }

    //ガイドブロック更新
    GuideUpdate();
}

void Player::Render()
{
    model->Render();
    for (auto& projectile : projectiles)
    {
        projectile->Render();
    }

    for (auto& effect : projectilePopEffects)
    {
        effect->Render();
    }

    auto& graphics{ Regal::Graphics::Graphics::Instance() };

    if (useBlockGroup)
    {
        graphics.SetRSState(Regal::Graphics::RASTER_STATE::WIREFRAME);
        for (int i = 0; i < 4; i++)
        {
            guideBlock->GetBlocks(i).Render();
        }
    }

    graphics.Set2DStates();
    

    if (healedTimer < 4)
    {
        hpHealGauge->Render();
    }

    hpGauge->Render();

    

    graphics.Set3DStates();
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

    hpGauge->DrawDebug();

    hpHealGauge->DrawDebug();

    if (ImGui::BeginMenu("Guide"))
    {
        guideBlock->DrawDebug();
        ImGui::EndMenu();
    }

    ImGui::End();
    
}

void Player::UseBlocksMove(float elapsedTime)
{
    //操作対象のブロックがなければ処理しない
    if (!useBlockGroup)return;

    //上入力で底に最速ブロック配置
    if (MoveUpButton())
    {
        while (useBlockGroup->MoveDown(1));
        autoSetTimer = autoSetTime;
    }

    //上方向以外への移動
    if (MoveBottomRightButton() || FastMoveBottomRightButton(elapsedTime))
    {
        if (useBlockGroup->MoveBottomRight(1))
        {
            //自動落下時間リセット
            autoFallTimer = 0;

            //自動設置時間リセット
            autoSetTimer = 0;
        }
    }
    else if (MoveBottomLeftButton() || FastMoveBottomLeftButton(elapsedTime))
    {
        if (useBlockGroup->MoveBottomLeft(1))
        {
            //自動落下時間リセット
            autoFallTimer = 0;

            //自動設置時間リセット
            autoSetTimer = 0;
        }
    }
    else if (MoveDownButton() || FastMoveDownButton(elapsedTime))
    {
        if (useBlockGroup->MoveDown(1))
        {
            //自動落下時間リセット
            autoFallTimer = 0;
        }
    }
    else if (MoveRightButton() || FastMoveRightButton(elapsedTime) && !Regal::Input::Keyboard::GetKeyState().LeftShift)
    {
        useBlockGroup->MoveRight(1);
    }
    else if (MoveLeftButton() || FastMoveLeftButton(elapsedTime) && !Regal::Input::Keyboard::GetKeyState().LeftShift)
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
        auto* bg{ useBlockGroup };
        autoSetTimer = 0;

        ChangeUseBG();

        //ブロックが制限ラインに達しているなら盤面を全てリセット
        bg->OutFrame();
    }
}

void Player::ChangeUseBG()
{
    GameManager::Instance().NextBlockUse();
}

bool Player::ApplyDamage(int damage)
{
    //ダメージが０の場合は健康状態を変更する必要がない
    if (damage == 0)return false;

    //死亡している場合は健康状態を変更しない
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
    AudioManager::Instance().Play(AudioManager::DAMAGED);
    Regal::Game::Camera::Instance().ScreenVibrate(0.08f,0.7f);
    hpGauge->SetColor(DirectX::XMFLOAT4(1, 0, 0, 1));
    hpGaugeDamageTimer = 0.0f;
}

void Player::OnDead()
{
    AudioManager::Instance().Play(AudioManager::LOSE);
    Regal::Game::Camera::Instance().ScreenVibrate(0.2f,3.0f);
    GameManager::Instance().GameClear();
}

void Player::OnHealed()
{
    hpGauge->SetColor(DirectX::XMFLOAT4(0, 1, 0, 1));

    //回復専用のHPゲージのみ伸ばすための処理
    healedTimer = 0.0f;

    hpGaugeDamageTimer = 0.0f;
}

void Player::GuideUpdate()
{
    if (!useBlockGroup)return;
    
    for (int i = 0; i < 4; i++)
    {
        auto color = Block::GetTypeColor(useBlockGroup->GetBlocks(i).GetType());
        
        guideBlock->GetBlocks(i).GetModel()->SetEmissiveColor(color);

        guideBlock->GetBlocks(i).ConvertToWorldPos();
    }

    //設置可能な最低の位置に移動
    while (guideBlock->MoveDown(1));
    
    if (useBlockGroup->GetPosition)
    {

    }
}

void Player::ApplyHeal(int value)
{
    if (hp <= 0)return;

    //HPが最大値を越える場合は回復しない
    if (hp + value > maxHp)
    {
        hp = maxHp;
        return;
    }

    OnHealed();

    hp += value;
}

void Player::Shot(int type)
{
    auto p = new Projectile;
    p->owner = this;
    p->type = type;
    p->power = power[type];
    p->CreateResource();
    p->Initialize();
    projectiles.emplace_back(p);
}

void Player::ProjectilesClear()
{
    for (auto& projectile : projectiles)
    {
        delete projectile;
    }
    projectiles.clear();
}

void Player::Projectile::CreateResource()
{
    model = std::make_unique<Regal::Model::StaticModel>("./Resources/Models/LuminousCube04.fbx");
}

void Player::Projectile::Initialize()
{
    auto pos = owner->model->GetTransform().GetPosition();
    pos.x += 8;
    model->GetTransform().SetPosition(pos);
    model->GetTransform().SetScaleFactor(0);

    chargeTimer = 0;

    switch (type)
    {
    case Block::RED:
        model->SetEmissiveColor(DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
        break;
    case Block::CYAN:
        model->SetEmissiveColor(DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f));
        break;
    case Block::GREEN:
        model->SetEmissiveColor(DirectX::XMFLOAT4(0.4f, 1.0f, 0.0f, 1.0f));
        break;
    case Block::PURPLE:
        model->SetEmissiveColor(DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f));
        break;
    }
}

void Player::Projectile::Update(float elapsedTime)
{
    model->GetTransform().AddRotation(DirectX::XMFLOAT3(
        spinSpeed * elapsedTime, spinSpeed * elapsedTime, 0));

    if (completeCharge)
    {
        model->GetTransform().AddPosition(DirectX::XMFLOAT3(speed * elapsedTime, 0, 0));
    }
    else
    {
        scale = (chargeTimer / chargeTime) * (1.0f + power * 0.05f);

        spinSpeed = (chargeTimer / chargeTime) * 3.0f;

        if (chargeTimer > chargeTime)
        {
            AudioManager::Instance().Play(AudioManager::SHOT);
            completeCharge = true;
            chargeTimer = 0;
        }

        model->GetTransform().SetScaleFactor(scale);
        chargeTimer += elapsedTime;
    }

    //判定処理
    auto enemyPosX{ EnemyManager::Instance().GetEnemy()->GetTransform().GetPosition().x };
    if (model->GetTransform().GetPosition().x > enemyPosX - 9.0f)
    {
        Hit();
    }
}

void Player::Projectile::Render()
{
    //弾丸にはカリングオフのwireframeを表示させる
    Regal::Graphics::Graphics::Instance().SetRSState(Regal::Graphics::WIREFRAME_CULL_NONE);
    model->Render();
    Regal::Graphics::Graphics::Instance().Set3DStates();

    
}

void Player::Projectile::DrawDebug()
{
    model->DrawDebug();
}

void Player::Projectile::Hit()
{
    EnemyManager::Instance().GetEnemy()->ApplyDamage(power);

    owner->GetProjectilePopEffect(type)->SetScale(1.2f + power * 0.03f);
    owner->GetProjectilePopEffect(type)->Play(model->GetTransform().GetPosition());

    

    Remove();
}

void Player::Projectile::Remove()
{
    owner->GetRemoves().emplace_back(this);
}