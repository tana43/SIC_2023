#include "SceneManager.h"

namespace Regal::Scene
{
    void SceneManager::Initialize()
    {

    }

    void SceneManager::Update(float elapsedFlame)
    {
        //次のシーンがセットされているなら切り替え
        if (nextScene)
        {
            //古いシーンを終了
            Clear();

            //新しいシーンを設定
            currentScene = nextScene;
            nextScene = nullptr;

            //シーン初期化処理(マルチスレッド処理をしていない場合に行う)
            if (!currentScene->IsReady())
            {
                currentScene->CreateResource();
                currentScene->Initialize();
            }
        }

        //シーン更新
        if (currentScene)
        {
            currentScene->Begin();
            currentScene->Update(elapsedFlame);
            currentScene->End();
        }
#ifdef _DEBUG
        //各シーンのデバッグ表示
        currentScene->DrawDebug();

        //シーンマネージャーのデバッグ表示
        DrawDebug();
#endif // _DEBUG

    }

    void SceneManager::Render(float elapsedFlame)
    {
        if (!currentScene)return;
        
        currentScene->Render(elapsedFlame);
    }

    void SceneManager::DrawDebug()
    {

    }

    void SceneManager::Clear()
    {
        if (!currentScene)return;
        {
            currentScene->Finalize();
            delete currentScene;
        }
    }

    //シーン切り替え
    void SceneManager::ChangeScene(BaseScene* scene)
    {
        //新しいシーンを設定
        nextScene = scene;
    }
}