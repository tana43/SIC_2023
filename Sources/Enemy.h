#pragma once

#include "RegalLib/Regal.h"

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

        void Destroy(Projectile* projectile);

        float chargeTime{ 5.0f };
        float chargeTimer;

        float scaleFactor{ 0.01f };
        float spinSpeed{0.1f};

        bool completeCharge{false};

        Enemy* owner;
    };

    Enemy(int type = 0) :GameObject("Enemy"),type(type) {}
    ~Enemy() {}

    void CreateResource()override;
    void Initialize()override;
    void Update(float elapsedTime)override;
    void Render()override;
    void DrawDebug()override;

    void Shot();

    void Destroy();

    std::vector<Projectile*>& GetProjectiles() { return projectiles; }
    std::vector<Projectile*>& GetRemoves() { return removes; }

private:
    int hp;
    int power;
    float attackTimer;
    float attackCoolTime;

    int type;
    int state;

    std::unique_ptr<Regal::Model::StaticModel> model;

    std::vector<Projectile*> projectiles;
    std::vector<Projectile*> removes;
};

