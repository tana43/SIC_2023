#include "BlockManager.h"
#include <set>

BlockManager::~BlockManager()
{
    Clear();
}

void BlockManager::Initialize()
{
	Clear();
}

void BlockManager::Update(float elapsedTime)
{
	for (Block* block : blocks)
	{
		block->Update(elapsedTime);
	}

	//�j������
	//�͈̓x�[�Xfor���̒��Ŕj������ƕs����N����
	for (Block* block : removes)
	{
		//�C�e���[�^�[���炶��Ȃ��Ɣj���ł��Ȃ�
		std::vector<Block*>::iterator it =
			std::find(blocks.begin(), blocks.end(), block);

		if (it != blocks.end())
		{
			blocks.erase(it);
		}
		//�e�ۂ̔j������
		delete block;
	}
	//�j�����X�g�N���A
	removes.clear();
}

void BlockManager::Render()
{
	for (Block* block : blocks)
	{
		block->Render();
	}
}

void BlockManager::DrawDebug()
{
	if (ImGui::BeginMenu("BlockManager"))
	{
		if (ImGui::Button("Add"))
		{
			Register(new Block);
		}

		auto& blocks{ BlockManager::Instance().GetBlocks() };

		//�X�N���[���\�ȃ��X�g
		ImGui::BeginChild(ImGui::GetID((void*)0), ImVec2(250, 100), ImGuiWindowFlags_NoTitleBar);
		for (int i = 0; i < blocks.size(); ++i) 
		{
			std::string name = "Block" + std::to_string(i);
			if (ImGui::BeginMenu(name.c_str()))
			{
				blocks.at(i)->DrawDebug();
				ImGui::EndMenu();
			}
		}
		ImGui::EndChild();

		
		ImGui::EndMenu();
	}
}

void BlockManager::Clear()
{
	for (Block* block : blocks)
	{
		delete block;
	}
}

void BlockManager::Register(Block* block)
{
	blocks.emplace_back(block);
	block->CreateResource();
	block->Initialize();
}

void BlockManager::Remove(Block* block)
{
	removes.emplace_back(block);
}
