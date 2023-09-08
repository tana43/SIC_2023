#include "PuzzleFrame.h"
#include "BlockManager.h"

void PuzzleFrame::CreateResource()
{
    frameModel = std::make_unique<Regal::Model::StaticModel>("./Resources/Models/PazzleFrame03.fbx");
}

void PuzzleFrame::Initialize()
{
    Clear();

    //Transform各項目設定
    auto* transform{ frameModel->GetTransform() };
    //transform->SetRotationY(DirectX::XMConvertToRadians(90));
    transform->SetScaleFactor(0.7f);
    //transform->SetScale(DirectX::XMFLOAT3(0.5f, 1, 0.8f));
    transform->SetPosition(DirectX::XMFLOAT3(5.0f,-2.8f,0));

    //カメラ位置を設定
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
    if (ImGui::BeginMenu(name.c_str()))
    {
        ImGui::Begin(name.c_str());

        frameModel->DrawDebug();

        ImGui::End();

        ImGui::EndMenu();
    }
}

void PuzzleFrame::CheckBlocks()
{
    //一度枠の情報を全てリセット
    Clear();

    auto& blocks = BlockManager::Instance().GetBlocks();
    for (auto& block : blocks)
    {
        //設置されていないならcontinue
        if (!block->GetIsPlaced())continue;

        SetBlockOnGrid(block);
    }
}

bool PuzzleFrame::SetBlockOnGrid(Block* block)
{
    auto gridPos{ block->GetGridPos() };

    //設置できない
    if (!SetBlockDetection(gridPos.x, gridPos.y))return false;

    gridsState[gridPos.x][gridPos.y] = 1;
    gridsBlock[gridPos.x][gridPos.y] = block;

    return true;
}

bool PuzzleFrame::IsOverToleranceLimit()
{
    for (int y = 0; y < MAX_FRAME_HEIGHT; y++)
    {
        for (int x = 0; x < MAX_FRAME_WIDTH; x++)
        {
            //ブロックがあるところが範囲外かどうか
            if (gridsBlock[y][x])
            {
                if (gridsState[y][x] == OUT_RANGE)return true;
            }
            else continue;
        }
    }

    return false;
}

void PuzzleFrame::OverToleranceLimit()
{
    //全削除
    BlockManager::Instance().Clear();
}

bool PuzzleFrame::SetBlockDetection(int gridX, int gridY)
{
    if (gridX < 0 || gridX >= MAX_FRAME_WIDTH)return false;
    if (gridY < 0 || gridY >= MAX_FRAME_HEIGHT)return false;

    if (gridsState[gridY][gridX] == 0) return true;

    return false;
}

void PuzzleFrame::Clear()
{
    for (int y = 0; y < MAX_FRAME_HEIGHT; y++)
    {
        for (int x = 0; x < MAX_FRAME_WIDTH; x++)
        {
            //ブロック情報リセット
            gridsBlock[y][x] = nullptr;

            //ステート情報リセット
            gridsState[y][x] = INIT_GRID_STATES[y][x];
        }
    }
}
