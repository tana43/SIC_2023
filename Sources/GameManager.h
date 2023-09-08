#pragma once

//スコアの管理、ゲームの進行を担うクラス
class GameManager
{
private: 
    GameManager() {}
    ~GameManager() {}

public:
    static GameManager& Instance()
    {
        static GameManager instance;
        return instance;
    }

private:
    int score;
    int highScore;

    float timer;

    int stageLevel;
};

