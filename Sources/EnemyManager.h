#pragma once
#include "RegalLib/Game/GameObject.h"
#include <vector>

#include "Enemy.h"

class EnemyManager
{
private:
    EnemyManager() {}
    ~EnemyManager();

public:
    static EnemyManager& Instance()
    {
        static EnemyManager instance;
        return instance;
    }

    void Initialize();
    void Update(float elapsedTime);
    void Render();
    void DrawDebug();

    void Clear();

    //¶¬‚µ‚½ƒGƒlƒ~[‚Ì“o˜^
    void Register(Enemy* Enemy);

    void Remove(Enemy* Enemy);

    Enemy* GetEnemy() { return curEnemy; }

    void SetSpriteColor(const DirectX::XMFLOAT4 color) { hpGauge->SetColor(color.x, color.y, color.z, color.w); }

    void Change();

    void EffectPlay(Enemy* enemy);

private:
    Enemy* curEnemy;

    std::vector<Enemy*> enemys;
    std::vector<Enemy*> removes;

    std::unique_ptr<Regal::Resource::Sprite> hpGauge;
    std::unique_ptr<Regal::Resource::Sprite> hpGaugeRest;
    DirectX::XMFLOAT2 spritePos{150,40};

    std::unique_ptr<PopEffect> popEffect;
};

