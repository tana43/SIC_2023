#include "GameManager.h"
#include "BlockGroupManager.h"

//スコアの管理、ゲームの進行を担うクラス

void GameManager::CreateResource()
{

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
}

void GameManager::Update(float elapsedTime)
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

void GameManager::DrawDebug()
{
    if (ImGui::BeginMenu(name.c_str()))
    {
        player->DrawDebug();

        ImGui::DragFloat("nBlock Interval", &nBlockInterval, 0.1f);
        ImGui::DragFloat3("nBlock Positoin", &nBlockPos.x, 0.1f);

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

    //ブロックがなめらかに動いてるように見せたいだけ
    //nBlockInterval = OFFSET_Y;
}
