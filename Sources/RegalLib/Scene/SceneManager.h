#pragma once
#include "BaseScene.h"

namespace Regal::Scene
{
    class SceneManager
    {
    private:
        SceneManager() {}
        ~SceneManager() {}

    public:
        static SceneManager& Instance()
        {
            static SceneManager instance;
            return instance;
        }

        void Update(float elapsedFlame);
        void Render(float elapsedFlame);

        void DrawDebug();
        void Clear();

        //ƒV[ƒ“Ø‚è‘Ö‚¦
        void ChangeScene(BaseScene* scene);

        BaseScene* GetCurrentScene()const { return currentScene; }

    private:
        BaseScene* currentScene = nullptr;
        BaseScene* nextScene = nullptr;
    };
}

