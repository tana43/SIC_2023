#pragma once

//�X�R�A�̊Ǘ��A�Q�[���̐i�s��S���N���X
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

