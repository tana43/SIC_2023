#pragma once

#include "RegalLib/Regal.h"
#include "PopEffect.h"

class Enemy : public Regal::Game::GameObject
{
public:
    enum Type
    {
        BLUE,
        ORANGE,
        PURPLE,
        RED,
        YELLOW,
        END,
    };

    enum State
    {
        IDLE,
        ATTACK,
        DIE,
    };

    struct Projectile
    {
        float speed{50.0f};
        std::unique_ptr<Regal::Model::StaticModel> model;

        void CreateResource();
        void Initialize();
        void Update(float elapsedTime);
        void Render();
        void DrawDebug();
        void Hit();

        void Remove(Projectile* projectile);

        float chargeTime{ 5.0f };
        float chargeTimer;

        float scaleFactor{ 0 };
        float spinSpeed{0.1f};

        bool completeCharge{false};

        int power{3};

        Enemy* owner;
    };

    Enemy(int type = 0) :GameObject("Enemy"),type(type) {}
    ~Enemy();

    void CreateResource()override;
    void Initialize()override;
    void Update(float elapsedTime)override;
    void Render()override;
    void DrawDebug()override;

    void Shot();

    void Remove();

    void ProjectilesClear();

    std::vector<Projectile*>& GetProjectiles() { return projectiles; }
    std::vector<Projectile*>& GetRemoves() { return removes; }

    bool ApplyDamage(int damage);

    void OnDamaged();
    void OnDead();

    int GetHp() { return hp; }
    int GetMaxHp() { return maxHp; }

    PopEffect* GetPopEffect() { return projectilePopEffect.get(); }

    Regal::Game::Transform& GetTransform() { return model->GetTransform(); }

    const int GetType() const { return type; }

    Regal::Model::StaticModel* GetModel() { return model.get(); }

private:
    int hp;
    int maxHp;
    int power;
    float attackTimer;
    float attackCoolTime;

    float deadTimer;

    float eIntensity{1.5f};

    float timer;

    int type;
    int state;

    std::unique_ptr<Regal::Model::StaticModel> model;

    std::vector<Projectile*> projectiles;
    std::vector<Projectile*> removes;

    std::unique_ptr<PopEffect> projectilePopEffect;

public:
    bool isDead{ false };
};

