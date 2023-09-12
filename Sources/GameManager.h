#pragma once
#include "Player.h"

//スコアの管理、ゲームの進行を担うクラス
class GameManager : Regal::Game::GameObject
{
private: 
    GameManager() : GameObject("GameManager") 
    {
        player = std::make_unique<Player>();
    }
    ~GameManager() {}

public:
    static constexpr float OFFSET_Y = 3.0f;

    static GameManager& Instance()
    {
        static GameManager instance;
        return instance;
    }

    void CreateResource()override;
    void Initialize()override;
    void Update(float elapsedTime)override;
    void DrawDebug()override;
    void Render()override;

    static Player& GetPlayer() { return *Instance().player; }
   

    void NextBlockUse();

private:
    int score;
    int highScore;

    float timer;

    int stageLevel;

    std::unique_ptr<Player> player;

    BlockGroup* nextBlockGroups[4];

    float nBlockInterval{10};
    DirectX::XMFLOAT3 nBlockPos{38,50,0};
};

