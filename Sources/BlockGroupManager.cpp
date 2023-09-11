#include "BlockGroupManager.h"
#include <set>

BlockGroupManager::~BlockGroupManager()
{
	Clear();
}

void BlockGroupManager::Initialize()
{
	Clear();
}

void BlockGroupManager::Update(float elapsedTime)
{
	for (BlockGroup* blockGroup : blockGroups)
	{
		blockGroup->Update(elapsedTime);
	}

	//�j������
	//�͈̓x�[�Xfor���̒��Ŕj������ƕs����N����
	for (BlockGroup* blockGroup : removes)
	{
		//�C�e���[�^�[���炶��Ȃ��Ɣj���ł��Ȃ�
		std::vector<BlockGroup*>::iterator it =
			std::find(blockGroups.begin(), blockGroups.end(), blockGroup);

		if (it != blockGroups.end())
		{
			blockGroups.erase(it);
		}
		//�u���b�N�̔j������
		delete blockGroup;
	}
	//�j�����X�g�N���A
	removes.clear();
}

void BlockGroupManager::Render()
{
	for (BlockGroup* blockGroup : blockGroups)
	{
		blockGroup->Render();
	}
}

void BlockGroupManager::DrawDebug()
{
	if (ImGui::BeginMenu("BlockGroupManager"))
	{
		if (ImGui::Button("Add"))
		{
			Register(new BlockGroup);
		}

		auto& blockGroups{ BlockGroupManager::Instance().GetBlockGroups() };

		//�X�N���[���\�ȃ��X�g
		ImGui::BeginChild(ImGui::GetID((void*)0), ImVec2(250, 100), ImGuiWindowFlags_NoTitleBar);
		for (int i = 0; i < blockGroups.size(); ++i)
		{
			std::string name = "BlockGroup" + std::to_string(i);
			if (ImGui::BeginMenu(name.c_str()))
			{
				blockGroups.at(i)->DrawDebug();
				ImGui::EndMenu();
			}
		}
		ImGui::EndChild();


		ImGui::EndMenu();
	}
}

void BlockGroupManager::Clear()
{
	for (BlockGroup* BlockGroup : blockGroups)
	{
		delete BlockGroup;
	}
	blockGroups.clear();
}

void BlockGroupManager::Register(BlockGroup* BlockGroup)
{
	blockGroups.emplace_back(BlockGroup);
	BlockGroup->CreateResource();
	BlockGroup->Initialize();
}

void BlockGroupManager::Destroy(BlockGroup* BlockGroup)
{
	removes.emplace_back(BlockGroup);
}
