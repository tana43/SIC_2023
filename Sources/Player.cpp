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
        L"./Resources/Images/PlayerHpGauge.png");
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

    model->GetTransform()->SetPositionX(-45.0f);
    model->GetTransform()->SetPositionY(72.0f);
    model->GetTransform()->SetScaleFactor(2.3f);

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
    
}

void Player::Update(float elapsedTime)
{
    //useBlockGroup->Update(elapsedTime);

    model->GetTransform()->AddRotationY(-0.1f * elapsedTime);
    model->GetTransform()->AddRotationX(0.1f * elapsedTime);

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
        model->GetSkinnedMesh()->SetEmissiveIntensity(intensity);
        blinkTimer += elapsedTime;
    }
    else
    {
        model->GetSkinnedMesh()->SetEmissiveIntensity(0);
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

    float screenCorrection{ graphics.GetScreenWidth() / 1280.0f };
    graphics.Set2DStates();
    hpGauge->_Render(graphics.GetDeviceContext(),
        spritePos.x, spritePos.y,
        300.0f * (static_cast<float>(hp) / static_cast<float>(maxHp)) * screenCorrection, 
        10.0f * screenCorrection, 0, 0,
        300.0f, 10, 0);
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

    ImGui::DragFloat2("Sprite Pos", &spritePos.x);

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
}

void Player::OnDead()
{
    AudioManager::Instance().Play(AudioManager::LOSE);
    Regal::Game::Camera::Instance().ScreenVibrate(0.2f,3.0f);
    GameManager::Instance().GameClear();
}

void Player::Heal(int value)
{
    if (hp <= 0)return;
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
    auto pos = owner->model->GetTransform()->GetPosition();
    pos.x += 8;
    model->GetTransform()->SetPosition(pos);
    model->GetTransform()->SetScaleFactor(0);

    chargeTimer = 0;

    switch (type)
    {
    case Block::RED:
        model->GetSkinnedMesh()->SetEmissiveColor(DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
        break;
    case Block::CYAN:
        model->GetSkinnedMesh()->SetEmissiveColor(DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f));
        break;
    case Block::GREEN:
        model->GetSkinnedMesh()->SetEmissiveColor(DirectX::XMFLOAT4(0.4f, 1.0f, 0.0f, 1.0f));
        break;
    case Block::PURPLE:
        model->GetSkinnedMesh()->SetEmissiveColor(DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f));
        break;
    }
}

void Player::Projectile::Update(float elapsedTime)
{
    model->GetTransform()->AddRotation(DirectX::XMFLOAT3(
        spinSpeed * elapsedTime, spinSpeed * elapsedTime, 0));

    if (completeCharge)
    {
        model->GetTransform()->AddPosition(DirectX::XMFLOAT3(speed * elapsedTime, 0, 0));
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

        model->GetTransform()->SetScaleFactor(scale);
        chargeTimer += elapsedTime;
    }

    //判定処理
    auto enemyPosX{ EnemyManager::Instance().GetEnemy()->GetTransform()->GetPosition().x };
    if (model->GetTransform()->GetPosition().x > enemyPosX - 9.0f)
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
    owner->GetProjectilePopEffect(type)->Play(model->GetTransform()->GetPosition());

    

    Remove();
}

void Player::Projectile::Remove()
{
    owner->GetRemoves().emplace_back(this);
}