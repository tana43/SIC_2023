#include "PuzzleFrame.h"
#include "BlockManager.h"

void PuzzleFrame::CreateResource()
{
    frameModel = std::make_unique<Regal::Model::StaticModel>("./Resources/Models/PazzleFrame03.fbx");
}

void PuzzleFrame::Initialize()
{
    Clear();

    //Transform�e���ڐݒ�
    auto* transform{ frameModel->GetTransform() };
    //transform->SetRotationY(DirectX::XMConvertToRadians(90));
    transform->SetScaleFactor(0.75f);
    //transform->SetScale(DirectX::XMFLOAT3(0.5f, 1, 0.8f));
    transform->SetPosition(DirectX::XMFLOAT3(2.1f,1.2f,0));

    //�J�����ʒu��ݒ�
    {
        auto* transform{ frameModel->GetTransform() };
        auto* cTransform{ Regal::Game::Camera::Instance().GetTransform() };

        DirectX::XMFLOAT3 eyePos;
        eyePos = transform->GetPosition();
        eyePos.y += 30.0f;
        eyePos.z -= 70.0f;
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
    //��x�g�̏���S�ă��Z�b�g
    Clear();

    auto& blocks = BlockManager::Instance().GetBlocks();
    for (auto& block : blocks)
    {
        //�ݒu����Ă��Ȃ��Ȃ�continue
        if (!block->GetIsPlaced())continue;

        SetBlockOnGrid(block);
    }
}

bool PuzzleFrame::SetBlockOnGrid(Block* block)
{
    auto gridPos{ block->GetGridPos() };

    //�ݒu�ł��Ȃ�
   /* if (!SetBlockDetection(gridPos.x, gridPos.y))
    {
        return false;
    }*/

    gridsState[gridPos.y][gridPos.x] = 1;
    gridsBlock[gridPos.y][gridPos.x] = block;

    return true;
}

bool PuzzleFrame::IsOverToleranceLimit()
{
    for (int y = 0; y < MAX_FRAME_HEIGHT; y++)
    {
        for (int x = 0; x < MAX_FRAME_WIDTH; x++)
        {
            //�u���b�N������Ƃ��낪�͈͊O���ǂ���
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
    //�S�폜
    BlockManager::Instance().Clear();
}

bool PuzzleFrame::SetBlockDetection(int gridX, int gridY)
{
    if (gridX < 0 || gridX >= MAX_FRAME_WIDTH)return false;
    if (gridY < 0 || gridY >= MAX_FRAME_HEIGHT)return false;

    if (gridsState[gridY][gridX] == NONE) return true;

    return false;
}

bool PuzzleFrame::MoveBlockDetection(int gridX, int gridY)
{
    if (gridX < 0 || gridX >= MAX_FRAME_WIDTH)return false;
    if (gridY < 0 || gridY >= MAX_FRAME_HEIGHT)return false;

    if (gridsState[gridY][gridX] == NONE || gridsState[gridY][gridX] == LIMIT) return true;

    return false;
}

//bool PuzzleFrame::IsBlockOnButtom(int gridX, int gridY)
//{
//    gridX -= 1;
//    gridY -= 1;
//    if (gridX < 0)return true;
//    if (gridY < 0)return true;
//
//    if (gridsState[gridY][gridX] == ON_BLOCK || gridsState[gridY][gridX] == OUT_RANGE) return true;
//
//    return false;
//}

void PuzzleFrame::Clear()
{
    for (int y = 0; y < MAX_FRAME_HEIGHT; y++)
    {
        for (int x = 0; x < MAX_FRAME_WIDTH; x++)
        {
            //�u���b�N��񃊃Z�b�g
            gridsBlock[y][x] = nullptr;

            //�X�e�[�g��񃊃Z�b�g
            gridsState[y][x] = INIT_GRID_STATES[y][x];
        }
    }
}
