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

	//破棄処理
	//範囲ベースfor文の中で破棄すると不具合が起こる
	for (BlockGroup* blockGroup : removes)
	{
		//イテレーターからじゃないと破棄できない
		std::vector<BlockGroup*>::iterator it =
			std::find(blockGroups.begin(), blockGroups.end(), blockGroup);

		if (it != blockGroups.end())
		{
			blockGroups.erase(it);
		}
		//ブロックの破棄処理
		delete blockGroup;
	}
	//破棄リストクリア
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

		//スクロール可能なリスト
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
