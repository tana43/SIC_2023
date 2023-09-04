#include "SceneManager.h"
#include "../../../External/imgui/imgui.h"

namespace Regal::Scene
{
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

    }

    void SceneManager::Render(float elapsedTime)
    {
        if (!currentScene)return;
        
        currentScene->Render(elapsedTime);
    }

    void SceneManager::DrawDebug()
    {
        if (!currentScene)return;

        //各シーンのデバッグ表示
        if (ImGui::BeginMenu("Scene"))
        {
            currentScene->DrawDebug();
            ImGui::EndMenu();
        }
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