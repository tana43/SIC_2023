#include "GameManager.h"
#include "BlockGroupManager.h"
#include "EnemyManager.h"
#include "BaseColorController.h"
#include "TitleScene.h"
#include "AudioManager.h"

//スコアの管理、ゲームの進行を担うクラス

void GameManager::CreateResource()
{
    player->CreateResource();
    sStageLevel = std::make_unique<Regal::Resource::Sprite>(Regal::Graphics::Graphics::Instance().GetDevice(),
        L"./Resources/Images/StageLevel.png");
    sGameClear = std::make_unique<Regal::Resource::Sprite>(Regal::Graphics::Graphics::Instance().GetDevice(),
        L"./Resources/Images/GameClear.png");
    number = std::make_unique<Numbers>();
}

void GameManager::Initialize()
{
    player->Initialize();

    for (auto& group : nextBlockGroups)
    {
        group = new BlockGroup(false);
        BlockGroupManager::Instance().Register(group);
    }

    //プレイヤーに使用ブロックセット
    NextBlockUse();

    EnemyManager::Instance().Register(new Enemy);

    stageLevel = 0;
    number->SetNumbers(stageLevel);
    number->SetPosition(DirectX::XMFLOAT2(1420.0f, 720.0f));
    number->SetScale(1.3f);
    number->SetBetweenNum(32);

    sAlpha = 0;

    gameClear = false;

    sStageLevel->GetSpriteTransform().SetPosition(DirectX::XMFLOAT2(180,90));
    sStageLevel->GetSpriteTransform().SetScale(1.3f);
}

void GameManager::Update(float elapsedTime)
{
    if (!gameClear)
    {

    player->Update(elapsedTime);

        for (int i = 0;i < 4;++i)
        {
            nextBlockGroups[i]->SetPosition(DirectX::XMFLOAT3(
            nBlockPos.x, nBlockPos.y - nBlockInterval * i,nBlockPos.z
            ));

            //nextBlockGroups[i]->Update(elapsedTime);
        }

    }
    else
    {
        if (sAlpha < 1)
        {
            sAlpha += elapsedTime * 0.3f;
        }
        else
        {
            if (Player::SelectButton())
            {
                Regal::Scene::SceneManager::Instance().ChangeScene(new TitleScene);
            }
        }
        sGameClear->SetColor(rundomColor.x, rundomColor.y, rundomColor.z, sAlpha);
        number->SetAlpha(sAlpha);
        number->SetPosition(DirectX::XMFLOAT2(552.0f, 179.0f));
        number->SetScale(1.53f);
        
    }
    number->SetNumbers(stageLevel);
}

void GameManager::DrawDebug()
{
    if (ImGui::BeginMenu(name.c_str()))
    {
        player->DrawDebug();

        ImGui::DragFloat("nBlock Interval", &nBlockInterval, 0.1f);
        ImGui::DragFloat3("nBlock Positoin", &nBlockPos.x, 0.1f);
        
        number->DrawDebug();

        sStageLevel->DrawDebug();
        sGameClear->DrawDebug();

        //スクロール可能なリスト
        ImGui::BeginChild(ImGui::GetID((void*)0), ImVec2(250, 100), ImGuiWindowFlags_NoTitleBar);
        for (int i = 0; i < 4; ++i)
        {
            std::string name = "BlockGroup" + std::to_string(i);
            if (ImGui::BeginMenu(name.c_str()))
            {
                nextBlockGroups[i]->DrawDebug();
                ImGui::EndMenu();
            }
        }
        ImGui::EndChild();

        ImGui::EndMenu();
    }
}

void GameManager::Render()
{
    player->Render();

    if (!gameClear)
    {
        auto& graphics{ Regal::Graphics::Graphics::Instance() };
        graphics.Set2DStates();
        sStageLevel->Render();

        number->Render();
        graphics.Set3DStates();
    }
}

void GameManager::GameClearRender()
{
    if (gameClear)
    {
        sGameClear->Render();

        number->Render();
    }
}

void GameManager::NextBlockUse()
{
    //プレイヤーが次のブロックを操作できるようにする
    nextBlockGroups[0]->Initialize();
    player->SetUseBlockGroup(nextBlockGroups[0]);

    //次のブロックを１つ上げる
    nextBlockGroups[0] = nextBlockGroups[1];
    nextBlockGroups[1] = nextBlockGroups[2];
    nextBlockGroups[2] = nextBlockGroups[3];
    nextBlockGroups[3] = new BlockGroup(false);
    BlockGroupManager::Instance().Register(nextBlockGroups[3]);

    //生成位置に描画されてしまうのを防止
    for (int i = 0; i < 4; ++i)
    {
        nextBlockGroups[i]->SetPosition(DirectX::XMFLOAT3(
            nBlockPos.x, nBlockPos.y - nBlockInterval * i, nBlockPos.z
        ));
        nextBlockGroups[i]->Update(0);
    }
    //ブロックがなめらかに動いてるように見せたいだけ
    //nBlockInterval = OFFSET_Y;
}

void GameManager::GameClear()
{
    gameClear = true;
    rundomColor = BaseColorController::GetRundomBrightColor();
    //AudioManager::Instance().GetSE(AudioManager::)->play();
}
