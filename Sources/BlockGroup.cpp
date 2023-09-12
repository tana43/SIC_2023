#include "BlockGroup.h"
#include "BlockManager.h"
#include "PuzzleFrame.h"
#include "BlockGroupManager.h"

BlockGroup::BlockGroup(bool onGrid) : GameObject("BlocksGroup"),onGrid(onGrid)
{
}

void BlockGroup::CreateResource()
{
	for (int i = 0; i < 4; i++)
	{
		blocks[i] = new Block(onGrid);
		BlockManager::Instance().Register(blocks[i]);
	}
}

void BlockGroup::Initialize()
{
	gridPos.x = GenerationPosX;
	gridPos.y = GenerationPosY;

	auto blockGridPos = gridPos;
	blocks[0]->SetGridPos(blockGridPos);

	blockGridPos = { gridPos.x + 1,gridPos.y };
	blocks[1]->SetGridPos(blockGridPos);

	blockGridPos = { gridPos.x + 1, gridPos.y + 1 };
	blocks[2]->SetGridPos(blockGridPos);

	blockGridPos = { gridPos.x, gridPos.y + 1 };
	blocks[3]->SetGridPos(blockGridPos);
}

void BlockGroup::Update(float elapsedTime)
{
	if (onGrid)
	{
		auto blockGridPos = gridPos;
		blocks[0]->SetGridPos(blockGridPos);

		blockGridPos = { gridPos.x + 1,gridPos.y };
		blocks[1]->SetGridPos(blockGridPos);

		blockGridPos = { gridPos.x + 1, gridPos.y + 1 };
		blocks[2]->SetGridPos(blockGridPos);

		blockGridPos = { gridPos.x, gridPos.y + 1 };
		blocks[3]->SetGridPos(blockGridPos);
	}
	else
	{

		static float root2 = sqrtf(2.0f);
		auto blockPos = position;
		blocks[0]->GetTransform()->SetPosition(blockPos);

		blockPos = { position.x + Block::blockInterval / root2 ,position.y + Block::blockInterval / root2,position.z };
		blocks[1]->GetTransform()->SetPosition(blockPos);

		blockPos = { position.x,position.y + Block::blockInterval * root2,position.z};
		blocks[2]->GetTransform()->SetPosition(blockPos);

		blockPos = { position.x - Block::blockInterval / root2 ,position.y + Block::blockInterval / root2,position.z };
		blocks[3]->GetTransform()->SetPosition(blockPos);
	}
}

void BlockGroup::Render()
{
	
}

void BlockGroup::DrawDebug()
{
	if (ImGui::BeginMenu(name.c_str()))
	{
		ImGui::Checkbox("On Grid", &onGrid);
		if (onGrid)
		{
			ImGui::SliderInt("GridPosX", &gridPos.x,0,PuzzleFrame::MAX_FRAME_WIDTH - 1);
			ImGui::SliderInt("GridPosY", &gridPos.y,0,PuzzleFrame::MAX_FRAME_HEIGHT - 1);
			if(ImGui::Button("Move Down"))
			{
				MoveDown(1);
			}
		}
		else
		{
			ImGui::DragFloat3("Position", &position.x, 0.1f);
		}
		ImGui::EndMenu();
	}
}

bool BlockGroup::MoveDown(int moveDistance)
{
	//ブロックが動いても枠の外に出ないかチェック
	for (auto& block : blocks)
	{
		if (!block->CanMoveDown(moveDistance))return false;
	}

	Block::GridPosition moveGridPos{
		gridPos.x - moveDistance,
		gridPos.y - moveDistance
	};
	moveGridPos.x = std::clamp(moveGridPos.x, 0, PuzzleFrame::MAX_FRAME_WIDTH);
	moveGridPos.y = std::clamp(moveGridPos.y, 0, PuzzleFrame::MAX_FRAME_HEIGHT);

	gridPos = moveGridPos;

	for (auto& block : blocks)
	{
		block->MoveDown(moveDistance);
	}

	return true;
}

void BlockGroup::MoveRight(int moveDistance)
{
	//ブロックが動いても枠の外に出ないかチェック
	for (auto& block : blocks)
	{
		if (!block->CanMoveRight(moveDistance))return;
	}

	Block::GridPosition moveGridPos{
		gridPos.x + moveDistance,
		gridPos.y - moveDistance
	};
	moveGridPos.x = std::clamp(moveGridPos.x, 0, PuzzleFrame::MAX_FRAME_WIDTH);
	moveGridPos.y = std::clamp(moveGridPos.y, 0, PuzzleFrame::MAX_FRAME_HEIGHT);

	gridPos = moveGridPos;

	for (auto& block : blocks)
	{
		block->MoveRight(moveDistance);
	}
}

void BlockGroup::MoveLeft(int moveDistance)
{
	//ブロックが動いても枠の外に出ないかチェック
	for (auto& block : blocks)
	{
		if (!block->CanMoveLeft(moveDistance))return;
	}

	Block::GridPosition moveGridPos{
		gridPos.x - moveDistance,
		gridPos.y + moveDistance
	};
	moveGridPos.x = std::clamp(moveGridPos.x, 0, PuzzleFrame::MAX_FRAME_WIDTH);
	moveGridPos.y = std::clamp(moveGridPos.y, 0, PuzzleFrame::MAX_FRAME_HEIGHT);

	gridPos = moveGridPos;

	for (auto& block : blocks)
	{
		block->MoveLeft(moveDistance);
	}
}

bool BlockGroup::MoveBottomRight(int moveDistance)
{
	//ブロックが動いても枠の外に出ないかチェック
	for (auto& block : blocks)
	{
		if (!block->CanMoveBottomRight(moveDistance))return false;
	}

	Block::GridPosition moveGridPos{
		gridPos.x,
		gridPos.y - moveDistance
	};
	moveGridPos.x = std::clamp(moveGridPos.x, 0, PuzzleFrame::MAX_FRAME_WIDTH);
	moveGridPos.y = std::clamp(moveGridPos.y, 0, PuzzleFrame::MAX_FRAME_HEIGHT);

	gridPos = moveGridPos;

	for (auto& block : blocks)
	{
		block->MoveBottomRight(moveDistance);
	}

	return true;
}

bool BlockGroup::MoveBottomLeft(int moveDistance)
{
	//ブロックが動いても枠の外に出ないかチェック
	for (auto& block : blocks)
	{
		if (!block->CanMoveBottomLeft(moveDistance))return false;
	}

	Block::GridPosition moveGridPos{
		gridPos.x - moveDistance,
		gridPos.y
	};
	moveGridPos.x = std::clamp(moveGridPos.x, 0, PuzzleFrame::MAX_FRAME_WIDTH);
	moveGridPos.y = std::clamp(moveGridPos.y, 0, PuzzleFrame::MAX_FRAME_HEIGHT);

	gridPos = moveGridPos;

	for (auto& block : blocks)
	{
		block->MoveBottomLeft(moveDistance);
	}

	return true;
}

void BlockGroup::RotRight()
{
	auto temp{ blocks[0] };

	blocks[0] = blocks[1];
	blocks[1] = blocks[2];
	blocks[2] = blocks[3];
	blocks[3] = temp;
}

void BlockGroup::RotLeft()
{
	auto temp{ blocks[2] };

	blocks[2] = blocks[1];
	blocks[1] = blocks[0];
	blocks[0] = blocks[3];
	blocks[3] = temp;
}

bool BlockGroup::IsBottom()
{
	char checkStates[5]{
		PuzzleFrame::Instance().GetGridState(blocks[0]->GetGridPos().x - 1, blocks[0]->GetGridPos().y - 1),
		PuzzleFrame::Instance().GetGridState(blocks[0]->GetGridPos().x - 1, blocks[0]->GetGridPos().y),
		PuzzleFrame::Instance().GetGridState(blocks[0]->GetGridPos().x, blocks[0]->GetGridPos().y - 1),
		PuzzleFrame::Instance().GetGridState(blocks[1]->GetGridPos().x, blocks[1]->GetGridPos().y - 1),
		PuzzleFrame::Instance().GetGridState(blocks[3]->GetGridPos().x - 1, blocks[3]->GetGridPos().y)
	};

	for (auto& state : checkStates)
	{
		if (state == PuzzleFrame::ON_BLOCK || state == PuzzleFrame::OUT_RANGE)return true;
	}
	return false;
}

void BlockGroup::PutOnGrid()
{
	for (auto& block : blocks)
	{
		PuzzleFrame::Instance().SetBlockOnGrid(block);
		block->PutOnGrid();
	}

	for (auto& block : blocks)
	{
		//連結しているブロックがないかチェック
		PuzzleFrame::Instance().CheckChainBlock(block);
	}

	BlockGroupManager::Instance().Remove(this);
}

const bool BlockGroup::CanMoveDown()
{
	//ブロックが下に動いても枠の外に出ないかチェック
	for (auto& block : blocks)
	{
		if (!block->CanMoveDown(1))return false;
	}
	return true;
}

void BlockGroup::OutFrame()
{
	for (auto& block : blocks)
	{
		//範囲外にブロックが出てしまった場合クリア
		if (PuzzleFrame::Instance().GetGridState(block->GetGridPos().x, block->GetGridPos().y) == PuzzleFrame::LIMIT)
		{
			PuzzleFrame::Instance().Clear();
		}
	}
}
