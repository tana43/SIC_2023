#include "PuzzleFrame.h"
#include "Easing.h"
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
    //�u���b�N�ǂ������珇�ɂ��邭�邳���悤��
    static float timer = 0;

    for (int y = 0; y < MAX_FRAME_HEIGHT; y++)
    {
        float time = timer + y * 0.1f;
        for (int x = 0; x < MAX_FRAME_WIDTH; x++)
        {
            if (gridsBlock[y][x] == nullptr)continue;

            if (time > 3.0f && time < 4.0f)
            {
                gridsBlock[y][x]->GetTransform()->SetRotationY(
                    Easing::InOutCubic(time - 3.0f, 1.0f/*���o����*/, DirectX::XM_PIDIV2 + DirectX::XM_PI, DirectX::XM_PIDIV2)
                );
            }
            else
            {
                gridsBlock[y][x]->GetTransform()->SetRotationY(DirectX::XM_PIDIV2);
            }
        }

        if (y == 0)
        {
            if (timer > 5.0f)
            {
                timer = 0;
            }
        }
    }

    timer += elapsedTime;
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

void PuzzleFrame::CheckChainBlock(Block* block)
{
    //�אڂ��Ă���u���b�N��S�Ē��ׂ�
    Block* blocks[8];
    blocks[0] = gridsBlock[block->GetGridPos().x + 1][block->GetGridPos().y + 1];//��
    blocks[2] = gridsBlock[block->GetGridPos().x + 1][block->GetGridPos().y];//�E��
    blocks[1] = gridsBlock[block->GetGridPos().x][block->GetGridPos().y + 1];//����
    blocks[3] = gridsBlock[block->GetGridPos().x - 1][block->GetGridPos().y + 1];//�E
    blocks[4] = gridsBlock[block->GetGridPos().x + 1][block->GetGridPos().y - 1];//��
    blocks[5] = gridsBlock[block->GetGridPos().x][block->GetGridPos().y - 1];//�E��
    blocks[6] = gridsBlock[block->GetGridPos().x - 1][block->GetGridPos().y];//����
    blocks[7] = gridsBlock[block->GetGridPos().x - 1][block->GetGridPos().y - 1];//��

    //�ċA��A�A������u���b�N���������Ă���ꍇ�͏�����ς���
    if (block->GetAbility())//���łɃA�r���e�B���t�^����Ă���ꍇ
    {
        for (auto& b : blocks)
        {
            if (block->GetType() != b->GetType())continue;
            if (block->GetAbility() == b->GetAbility())continue;

            if (b->GetAbility()) 
            {
                //�A�h���X���قȂ�A�r���e�B�������Ă���̂ŁA�ǂ��炩�ɓ�������
                std::vector<Block*> saBlocks;
                BlockManager::Instance().FindSameAbilityBlocks(b->GetAbility(),saBlocks);

                for (auto& saBlock : saBlocks)
                {
                    //�A�r���e�B����
                    saBlock->SetAbility(block->GetAbility());
                }
            }
        }

        return;
    }

    //��x���ċA���Ă��Ȃ�
    for (auto& b : blocks)
    {
        //�^�C�v�������ł͂Ȃ��Ȃ�continue
        if (b->GetType() != block->GetType())continue;

        //�אڂ��Ă���u���b�N�����łɃ`�F�C�����Ă���ꍇ�͂���ɒǉ�����
        if (b->GetAbility())
        {
            auto cAbility{ b->GetAbility() };
            block->SetAbility(cAbility);
        }
        else//�אڂ��Ă���u���b�N���`�F�C�����Ă��Ȃ�
        {
            auto cAbility{new ChainAbility};
            cAbility->type = block->GetType();

            block->SetAbility(cAbility);
            b->SetAbility(cAbility);
            chainAbilitys.emplace_back(cAbility);

            //�`�F�C�����͓��A�h���X�̃A�r���e�B���������u���b�N�̌��ŎZ�o����I�I
            //cAbility->chain = 2;

            //�ċA
            CheckChainBlock(b);
        }
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

    //�A�r���e�B�z�񃊃Z�b�g
    chainAbilitys.clear();
}
