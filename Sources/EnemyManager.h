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

    //���������G�l�~�[�̓o�^
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
    std::unique_ptr<Regal::Resource::Sprite> hpGaugeRest;//�_���[�W���������ۂɌォ�猸���Ă���HP�Q�[�W
    float  hpGaugeRestSpeed = 0.2f;//��̃Q�[�W�̌���X�s�[�h

    std::unique_ptr<PopEffect> popEffect;
};

