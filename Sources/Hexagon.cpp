#include "Hexagon.h"
#include "PuzzleFrame.h"

DirectX::XMFLOAT2 Hexagon::STARTING_POS = DirectX::XMFLOAT2(1.5f, 2.0f);

void Hexagon::CreateResource()
{
    model = std::make_unique<Regal::Model::StaticModel>("./Resources/Models/LuminousHexagon01.fbx");
}

void Hexagon::Initialize()
{
	model->GetTransform()->SetRotationY(DirectX::XMConvertToRadians(90));
}

void Hexagon::Update(float elapsedTime)
{
	if (onGround)//Ú’n‚µ‚Ä‚¢‚éê‡
	{
		
	}
	else
	{
		
	}
	ConvertToWorldPos();
}

void Hexagon::Render()
{
	model->Render();
}

void Hexagon::DrawDebug()
{
	ImGui::Begin(name.c_str());

	ImGui::SliderInt("GridPosX", &gridPos.x, 0, PuzzleFrame::MAX_FRAME_WIDTH);
	ImGui::SliderInt("GridPosY", &gridPos.y, 0, PuzzleFrame::MAX_FRAME_HEIGHT);

	ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
	model->DrawDebug();

	ImGui::End();
}

void Hexagon::ConvertToWorldPos()
{
	float x = STARTING_POS.x + gridPos.x * blockInterval.x;
	float y = STARTING_POS.y + gridPos.y * blockInterval.y;
	model->GetTransform()->SetPositionX(x);
	model->GetTransform()->SetPositionY(y);
}
