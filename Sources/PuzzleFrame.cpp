#include "PuzzleFrame.h"

void PuzzleFrame::CreateResource()
{
    frameModel = std::make_unique<Regal::Model::StaticModel>("./Resources/Models/PazzleFrame.fbx");
}

void PuzzleFrame::Initialize()
{
    memset(frameStates,0,sizeof(frameStates));

    //TransformŠe€–ÚÝ’è
    auto* transform{ frameModel->GetTransform() };
    /*transform->SetRotationY(DirectX::XMConvertToRadians(90));
    transform->SetScaleFactor(10.0f);
    transform->SetScale(DirectX::XMFLOAT3(0.5f, 1, 0.8f));
    transform->SetPositionX(17.6f);*/

    //ƒJƒƒ‰ˆÊ’u‚ðÝ’è
    {
        auto* transform{ frameModel->GetTransform() };
        auto* cTransform{ Regal::Game::Camera::Instance().GetTransform() };

        DirectX::XMFLOAT3 eyePos;
        eyePos = transform->GetPosition();
        eyePos.y += 20.0f;
        eyePos.z -= 60.0f;
        cTransform->SetPosition(eyePos);
    }
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
