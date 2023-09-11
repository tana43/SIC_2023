#include "Enemy.h"
#include "EnemyManager.h"
#include "GameManager.h"

void Enemy::CreateResource()
{
    model = std::make_unique<Regal::Model::StaticModel>("./Resources/Models/EnemyType01.fbx");
}

void Enemy::Initialize()
{
    switch (type)
    {
    case BLUE:
        model->GetSkinnedMesh()->SetEmissiveColor(DirectX::XMFLOAT4(0,0,1,1));
        break;
    case ORANGE:
        model->GetSkinnedMesh()->SetEmissiveColor(DirectX::XMFLOAT4(1,0.4f,0,1));
        break;
    case PURPLE:
        model->GetSkinnedMesh()->SetEmissiveColor(DirectX::XMFLOAT4(0.5f,0,1,1));
        break;
    case RED:
        model->GetSkinnedMesh()->SetEmissiveColor(DirectX::XMFLOAT4(1,0,1,1));
        break;
    case YELLOW:
        model->GetSkinnedMesh()->SetEmissiveColor(DirectX::XMFLOAT4(1,1,0,1));
        break;
    }
    hp = 1000;
    power = 3;
    attackTimer = 0;
    model->GetSkinnedMesh()->SetColor(DirectX::XMFLOAT4(0, 0, 0, 1));
    model->GetSkinnedMesh()->SetEmissiveIntensity(1.5f);

    model->GetTransform()->SetScaleFactor(8.0f);
    model->GetTransform()->SetPositionX(51.0f);
    model->GetTransform()->SetPositionY(73.0f);
}

void Enemy::Update(float elapsedTime)
{
    model->GetTransform()->AddRotationY(0.1f * elapsedTime);

    switch (state)
    {
    case IDLE:
        break;
    case ATTACK:
        break;
    case DIE:
        break;
    }

    for (auto& projectile : projectiles)
    {
        projectile->Update(elapsedTime);
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

void Enemy::Render()
{
    model->Render();

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

void Enemy::Destroy()
{
    EnemyManager::Instance().Destroy(this);
}

void Enemy::Projectile::CreateResource()
{
    model = std::make_unique<Regal::Model::StaticModel>("./Resources/Models/EnemyType01.fbx");
}

void Enemy::Projectile::Initialize()
{
    auto pos = owner->model->GetTransform()->GetPosition();
    pos.x -= 10;
    model->GetTransform()->SetPosition(pos);

    chargeTimer = 0;

    model->GetSkinnedMesh()->SetEmissiveColor(DirectX::XMFLOAT4(1, 0, 0, 1));
}

void Enemy::Projectile::Update(float elapsedTime)
{
    model->GetTransform()->AddRotation(DirectX::XMFLOAT3(
        spinSpeed * elapsedTime,spinSpeed * elapsedTime,0));

    if (completeCharge)
    {
        model->GetTransform()->AddPosition(DirectX::XMFLOAT3(-speed * elapsedTime,0,0));
    }
    else
    {
        scaleFactor = (chargeTimer / chargeTime) * 3.0f;

        spinSpeed = (chargeTimer / chargeTime) * 3.0f;

        if (chargeTimer > chargeTime)
        {
            completeCharge = true;
            chargeTimer = 0;
        }

        model->GetTransform()->SetScaleFactor(scaleFactor);
        chargeTimer += elapsedTime;
    }

    //判定処理
    auto playerPosX{ GameManager::Instance().GetPlayer().GetTransform()->GetPosition().x };
    if (model->GetTransform()->GetPosition().x < playerPosX + 6.0f)
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
    Destroy(this);
}

void Enemy::Projectile::Destroy(Projectile* projectile)
{
    owner->GetRemoves().emplace_back(projectile);
}

