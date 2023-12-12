#include "Enemy.h"
#include "EnemyManager.h"
#include "GameManager.h"
#include "AudioManager.h"
#include "BaseColorController.h"

Enemy::~Enemy()
{
    ProjectilesClear();
}

void Enemy::CreateResource()
{
    model = std::make_unique<Regal::Model::StaticModel>("./Resources/Models/EnemyType01.fbx");
    projectilePopEffect = std::make_unique<PopEffect>(10);
}

void Enemy::Initialize()
{
    ProjectilesClear();

    switch (type)
    {
    case BLUE:
        model->SetEmissiveColor(DirectX::XMFLOAT4(0,0,1,1));
        break;
    case ORANGE:
        model->SetEmissiveColor(DirectX::XMFLOAT4(1,0.4f,0,1));
        break;
    case PURPLE:
        model->SetEmissiveColor(DirectX::XMFLOAT4(0.5f,0,1,1));
        break;
    case RED:
        model->SetEmissiveColor(DirectX::XMFLOAT4(1,0,1,1));
        break;
    case YELLOW:
        model->SetEmissiveColor(DirectX::XMFLOAT4(1,1,0,1));
        break;
    }
    hp = 1000 + 500 * GameManager::Instance().GetStageLevel();
    maxHp = hp;
    power = 3 +  GameManager::Instance().GetStageLevel();
    attackTimer = 0;

    attackCoolTime = 8.0f;

    model->GetSkinnedMesh()->SetColor(DirectX::XMFLOAT4(0, 0, 0, 1));
    model->SetEmissiveIntensity(0);

    model->GetTransform().SetScaleFactor(8.0f);
    model->GetTransform().SetPositionX(51.0f);
    model->GetTransform().SetPositionY(73.0f);

    state = IDLE;

    projectilePopEffect->SetColor(DirectX::XMFLOAT4(1, 0.3f, 0.3f, 0.5f));
    projectilePopEffect->SetScale(1.5f);

    EnemyManager::Instance().SetSpriteColor(model->GetEmissiveColor());

    BaseColorController::Instance().ChangeColorBGParticle(model->GetEmissiveColor());
}

void Enemy::Update(float elapsedTime)
{
    model->GetTransform().AddRotationY(0.1f * elapsedTime);

    if (timer < 2.0f)
    {
        model->SetEmissiveIntensity(timer * eIntensity);
    }

    switch (state)
    {
    case IDLE:

        if (attackTimer > attackCoolTime)
        {
            state = ATTACK;
        }

        attackTimer += elapsedTime;
        break;
    case ATTACK:

        Shot();
        attackTimer = 0;
        state = IDLE;
        break;
    case DIE:

        model->SetEmissiveIntensity(eIntensity * (1.0f - deadTimer));

        if (deadTimer > 1.0f)
        {
            //エネミーを交代
            EnemyManager::Instance().Change();
            return;
        }

        deadTimer += elapsedTime;
        break;
    }

    for (auto& projectile : projectiles)
    {
        projectile->Update(elapsedTime);
    }

    projectilePopEffect->Update(elapsedTime);

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

    timer += elapsedTime;
}

void Enemy::Render()
{
    model->Render();

    projectilePopEffect->Render();

    for (auto& projectile : projectiles)
    {
        projectile->Render();
    }
}

void Enemy::DrawDebug()
{
    ImGui::Begin(name.c_str());

    if (ImGui::Button("Shot"))
    {
        Shot();
    }

    ImGui::SliderInt("HP", &hp, 0, 1000);

    model->DrawDebug();

    if (ImGui::BeginMenu("Projectile"))
    {
        if (projectiles.size() > 0)
        {
            projectiles.at(0)->DrawDebug();
        }

        ImGui::EndMenu();
    }

    ImGui::End();
}

void Enemy::Shot()
{
    auto p = new Projectile;
    p->owner = this;
    p->CreateResource();
    p->Initialize();
    projectiles.emplace_back(p);
}

void Enemy::Remove()
{
    EnemyManager::Instance().Remove(this);
}

void Enemy::ProjectilesClear()
{
    for (auto& projectile : projectiles)
    {
        delete projectile;
    }
    projectiles.clear();
}

bool Enemy::ApplyDamage(int damage)
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

void Enemy::OnDamaged()
{
    AudioManager::Instance().Play(AudioManager::DAMAGED);
    Regal::Game::Camera::Instance().ScreenVibrate(0.08f, 0.7f);
}

void Enemy::OnDead()
{
    AudioManager::Instance().Play(AudioManager::ENEMY_DIE);
    Regal::Game::Camera::Instance().ScreenVibrate(0.3f, 5.0f);
    state = DIE;
}

void Enemy::Projectile::CreateResource()
{
    model = std::make_unique<Regal::Model::StaticModel>("./Resources/Models/EnemyType01.fbx");
}

void Enemy::Projectile::Initialize()
{
    auto pos = owner->model->GetTransform().GetPosition();
    pos.x -= 13;
    model->GetTransform().SetPosition(pos);
    model->GetTransform().SetScaleFactor(0);

    chargeTimer = 0;

    model->SetEmissiveColor(DirectX::XMFLOAT4(1, 0, 0, 1));
}

void Enemy::Projectile::Update(float elapsedTime)
{
    model->GetTransform().AddRotation(DirectX::XMFLOAT3(
        spinSpeed * elapsedTime,spinSpeed * elapsedTime,0));

    if (completeCharge)
    {
        model->GetTransform().AddPosition(DirectX::XMFLOAT3(-speed * elapsedTime,0,0));
    }
    else
    {
        scaleFactor = (chargeTimer / chargeTime) * 2.0f;

        spinSpeed = (chargeTimer / chargeTime) * 3.0f;

        if (chargeTimer > chargeTime)
        {
            AudioManager::Instance().Play(AudioManager::SHOT);
            completeCharge = true;
            chargeTimer = 0;
        }

        model->GetTransform().SetScaleFactor(scaleFactor);
        chargeTimer += elapsedTime;
    }

    //判定処理
    auto playerPosX{ GameManager::Instance().GetPlayer().GetTransform().GetPosition().x };
    if (model->GetTransform().GetPosition().x < playerPosX + 6.0f)
    {
        Hit();
    }
}

void Enemy::Projectile::Render()
{
    //弾丸にはカリングオフのwireframeを表示させる
    Regal::Graphics::Graphics::Instance().SetRSState(Regal::Graphics::WIREFRAME_CULL_NONE);
    model->Render();
    Regal::Graphics::Graphics::Instance().Set3DStates();
}

void Enemy::Projectile::DrawDebug()
{
    model->DrawDebug();
}

void Enemy::Projectile::Hit()
{    
    GameManager::GetPlayer().ApplyDamage(owner->power);
    GameManager::GetPlayer().GetModel()->SetEmissiveIntensity(10.0f);

    owner->GetPopEffect()->Play(model->GetTransform().GetPosition());

    Remove(this);
}

void Enemy::Projectile::Remove(Projectile* projectile)
{
    owner->GetRemoves().emplace_back(projectile);
}

