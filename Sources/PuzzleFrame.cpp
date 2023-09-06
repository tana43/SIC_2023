#include "PuzzleFrame.h"

void PuzzleFrame::CreateResource()
{
    frameModel = std::make_unique<Regal::Model::StaticModel>("./Resources/Models/Stage02.fbx");
}

void PuzzleFrame::Initialize()
{
    memset(frameStates,0,sizeof(frameStates));
    frameModel->GetTransform()->SetRotationY(DirectX::XMConvertToRadians(90));
}

void PuzzleFrame::Update(float elapsedTime)
{

}

void PuzzleFrame::Render()
{
    frameModel->Render();
}

void PuzzleFrame::DrawDebug()
{
    ImGui::Begin(name.c_str());

    frameModel->DrawDebug();

    ImGui::End();
}

void PuzzleFrame::CheckBlocks()
{

}
