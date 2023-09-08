#include "BlocksGroup.h"
#include "BlockManager.h"

BlocksGroup::BlocksGroup() : GameObject("BlocksGroup")
{
	for (int i = 0; i < 4; i++)
	{
		blocks[i] = new Block;
		BlockManager::Instance().Register(blocks[i]);
	}
}

void BlocksGroup::CreateResource()
{

}

void BlocksGroup::Initialize()
{
	for (auto& block :blocks)
	{
		block->Initialize();
	}
}

void BlocksGroup::Update(float elapsedTime)
{
	for (auto& block : blocks)
	{
		block->Update(elapsedTime);
	}
}

void BlocksGroup::DrawDebug()
{
	if (ImGui::BeginMenu(name.c_str()))
	{
		for (auto& block : blocks)
		{
			block->DrawDebug();
		}
		ImGui::EndMenu();
	}
}
