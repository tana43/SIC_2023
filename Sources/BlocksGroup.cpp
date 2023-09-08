#include "BlocksGroup.h"
#include "BlockManager.h"
#include "PuzzleFrame.h"

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
	gridPos.x = GenerationPosX;
	gridPos.y = GenerationPosY;

	auto blockGridPos = gridPos;
	blocks[0]->SetGridPos(gridPos);

	blockGridPos = { gridPos.x - 1,gridPos.y };
	blocks[1]->SetGridPos(gridPos);

	blockGridPos = { gridPos.x, gridPos.y - 1};
	blocks[2]->SetGridPos(gridPos);

	blockGridPos = { gridPos.x - 1, gridPos.y - 1};
	blocks[3]->SetGridPos(gridPos);
}

void BlocksGroup::Update(float elapsedTime)
{
	auto blockGridPos = gridPos;
	blocks[0]->SetGridPos(gridPos);

	blockGridPos = { gridPos.x - 1,gridPos.y };
	blocks[1]->SetGridPos(gridPos);

	blockGridPos = { gridPos.x, gridPos.y - 1 };
	blocks[2]->SetGridPos(gridPos);

	blockGridPos = { gridPos.x - 1, gridPos.y - 1 };
	blocks[3]->SetGridPos(gridPos);
}

void BlocksGroup::Render()
{
	
}

void BlocksGroup::DrawDebug()
{
	if (ImGui::BeginMenu(name.c_str()))
	{
		ImGui::SliderInt("GridPosX", &gridPos.x,0,PuzzleFrame::MAX_FRAME_WIDTH);
		ImGui::SliderInt("GridPosY", &gridPos.y,0,PuzzleFrame::MAX_FRAME_HEIGHT);
		ImGui::EndMenu();
	}
}
