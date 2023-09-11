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

    //ê∂ê¨ÇµÇΩÉGÉlÉ~Å[ÇÃìoò^
    void Register(Enemy* Enemy);

    void Destroy(Enemy* Enemy);

    Enemy* GetEnemy() { return enemys.at(0); }

private:
    std::vector<Enemy*> enemys;
    std::vector<Enemy*> removes;
};

