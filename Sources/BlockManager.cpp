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

	//破棄処理
	//範囲ベースfor文の中で破棄すると不具合が起こる
	for (Block* block : removes)
	{
		//イテレーターからじゃないと破棄できない
		std::vector<Block*>::iterator it =
			std::find(blocks.begin(), blocks.end(), block);

		if (it != blocks.end())
		{
			blocks.erase(it);
		}
		//ブロックの破棄処理
		delete block;
	}
	//破棄リストクリア
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

		//スクロール可能なリスト
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
	blocks.clear();
}

void BlockManager::Register(Block* block)
{
	blocks.emplace_back(block);
	block->CreateResource();
	block->Initialize();
}

void BlockManager::Destroy(Block* block)
{
	removes.emplace_back(block);
}

void BlockManager::FindSameAbilityBlocks(PuzzleFrame::ChainAbility* cAbility, std::vector<Block*>& bArray)
{
	for (Block* block : blocks)
	{
		if (block->GetAbility() == cAbility)bArray.emplace_back(block);
	}
}
