#include "Block.h"
#include "PuzzleFrame.h"

DirectX::XMFLOAT2 Block::STARTING_POS = DirectX::XMFLOAT2(5.0f, 2.0f);

Block::Block(int type) : GameObject("GameObject"),type(type)
{
	if (type == -1)this->type = rand() % BlockType::END;
}

void Block::CreateResource()
{
    //model = std::make_unique<Regal::Model::StaticModel>("./Resources/Models/LuminousHexagon01.fbx");
    model = std::make_unique<Regal::Model::StaticModel>("./Resources/Models/LuminousCube04.fbx");
}

void Block::Initialize()
{
	model->GetTransform()->SetRotationX(DirectX::XMConvertToRadians(45));
	model->GetTransform()->SetRotationY(DirectX::XMConvertToRadians(90));
	model->GetTransform()->SetScale(DirectX::XMFLOAT3(0.8f,1,1));

	switch (type)
	{
	case BlockType::RED:
		model->GetSkinnedMesh()->SetColor(DirectX::XMFLOAT4(1.0f,0.0f,0.0f,1.0f));
		break;
	case BlockType::CYAN:
		model->GetSkinnedMesh()->SetColor(DirectX::XMFLOAT4(0.0f,1.0f,1.0f,1.0f));
		break;
	case BlockType::GREEN:
		model->GetSkinnedMesh()->SetColor(DirectX::XMFLOAT4(0.4f,1.0f,0.0f,1.0f));
		break;
	case BlockType::PURPLE:
		model->GetSkinnedMesh()->SetColor(DirectX::XMFLOAT4(1.0f,0.0f,1.0f,1.0f));
		break;
	}
}

void Block::Update(float elapsedTime)
{
	if (isPlaced)//Ú’n‚µ‚Ä‚¢‚éê‡
	{
		
	}
	else
	{
		
	}
	ConvertToWorldPos();
}

void Block::Render()
{
	model->Render();
}

void Block::DrawDebug()
{
	ImGui::Begin(name.c_str());

	ImGui::SliderInt("GridPosX", &gridPos.x, 0, PuzzleFrame::MAX_FRAME_WIDTH);
	ImGui::SliderInt("GridPosY", &gridPos.y, 0, PuzzleFrame::MAX_FRAME_HEIGHT);

	ImGui::DragFloat("STARTING_POS_X", &STARTING_POS.x,0.1f);
	ImGui::DragFloat("STARTING_POS_Y", &STARTING_POS.y,0.1f);

	ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
	model->DrawDebug();

	ImGui::End();
}

void Block::SetOnGrid()
{
	PuzzleFrame::Instance().SetBlockOnGrid(this);

	isPlaced = true;
}

void Block::ConvertToWorldPos()
{
	float x = STARTING_POS.x + gridPos.x * blockInterval;
	float y = STARTING_POS.y + gridPos.y * blockInterval;
	DirectX::XMFLOAT2 moveX{x* cosf(DirectX::XMConvertToRadians(45)), x * sinf(DirectX::XMConvertToRadians(45))};
	DirectX::XMFLOAT2 moveY{y* cosf(DirectX::XMConvertToRadians(135)), y * sinf(DirectX::XMConvertToRadians(135))};
	model->GetTransform()->SetPositionX(moveX.x + moveY.x);
	model->GetTransform()->SetPositionY(moveX.y + moveY.y);
}

bool Block::CanMoveDown(int moveDistance)
{
	GridPosition moveGridPos{ gridPos.x - moveDistance,gridPos.y - moveDistance };

	if (PuzzleFrame::Instance().SetBlockDetection(moveGridPos.x, moveGridPos.y))return false;

	return true;
}

void Block::MoveDown(int moveDistance)
{
	gridPos.x -= moveDistance; 
	gridPos.y -= moveDistance;
}
