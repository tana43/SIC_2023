#include "GameManager.h"
#include "BlockGroupManager.h"

//�X�R�A�̊Ǘ��A�Q�[���̐i�s��S���N���X

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

    //�v���C���[�Ɏg�p�u���b�N�Z�b�g
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

        //�X�N���[���\�ȃ��X�g
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
    //�v���C���[�����̃u���b�N�𑀍�ł���悤�ɂ���
    nextBlockGroups[0]->Initialize();
    player->SetUseBlockGroup(nextBlockGroups[0]);

    //���̃u���b�N���P�グ��
    nextBlockGroups[0] = nextBlockGroups[1];
    nextBlockGroups[1] = nextBlockGroups[2];
    nextBlockGroups[2] = nextBlockGroups[3];
    nextBlockGroups[3] = new BlockGroup(false);
    BlockGroupManager::Instance().Register(nextBlockGroups[3]);

    //�u���b�N���Ȃ߂炩�ɓ����Ă�悤�Ɍ�����������
    //nBlockInterval = OFFSET_Y;
}
