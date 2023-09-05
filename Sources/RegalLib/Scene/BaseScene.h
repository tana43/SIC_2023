#pragma once

namespace Regal::Scene
{
    enum SCENE_TYPE
    {
        TITLE,
        GAME,
        LOAD,
    };

    class BaseScene
    {
    public:
        BaseScene() {}
        virtual ~BaseScene() {}

        virtual void CreateResource()                 = 0; // 生成
        virtual void Initialize()                     = 0; // 初期化
        virtual void Finalize()                       = 0; // 終了化
        virtual void Begin()                          = 0; // 毎フレーム一番最初に呼ばれる
        virtual void Update(const float& elapsedTime) = 0; // 更新処理
        virtual void End()                            = 0; // 毎フレーム一番最後に呼ばれる
        virtual void Render(const float& elapsedTime) = 0; // 描画処理
        virtual void DrawDebug()                      = 0;
        virtual void PostEffectDrawDebug()            = 0;

    public:
        // 準備完了しているかどうか
        bool IsReady()const { return ready; }

        // 準備完了
        void SetReady() { ready = true; }

        // 今どのシーンなのか判定できるようにする為のもの
        void SetSceneType(int type) { sceneType = type; }
        int GetSceneType() { return sceneType; }

    private:
        bool ready = false;

        int sceneType = SCENE_TYPE::TITLE;
    };
}