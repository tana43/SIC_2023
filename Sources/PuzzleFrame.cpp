#include "PuzzleFrame.h"
#include "Easing.h"
#include "BlockManager.h"
#include "Block.h"
#include "GameManager.h"
#include "AudioManager.h"

void PuzzleFrame::CreateResource()
{
    frameModel = std::make_unique<Regal::Model::StaticModel>("./Resources/Models/PazzleFrame03.fbx");
    keepOutBarModel = std::make_unique<Regal::Model::StaticModel>("./Resources/Models/LuminousCube04.fbx");
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
        eyePos.y += 43.0f;
        eyePos.z -= 88.0f;
        cTransform->SetPosition(eyePos);
    }

    //�N���֎~�G���A�o�[
    keepOutBarModel->GetTransform()->SetPosition(DirectX::XMFLOAT3(2.2f,46,3));
    keepOutBarModel->GetTransform()->SetScale(DirectX::XMFLOAT3(22.0f,1,1));
    keepOutBarModel->GetSkinnedMesh()->SetEmissiveColor(DirectX::XMFLOAT4(1, 0, 0, 1));
}

void PuzzleFrame::Update(float elapsedTime)
{
    //�u���b�N�ǂ������珇�ɂ��邭�邳���悤��

    static float timer;
    if (!isFrameAttack)
    {
        for (int y = 0; y < MAX_FRAME_HEIGHT; y++)
        {
            float time = timer - 0.15f * y;
            for (int x = 0; x < MAX_FRAME_WIDTH; x++)
            {
                auto block = GetGridBlock(x, y);
                if (block == nullptr)continue;

                if (time > 5.0f && time < 5.5f)
                {
                    block->Spin();
                }

                if (time > 10.0f && time < 10.5f)
                {
                    block->Spin();
                }
            }
        }
    }

    if (timer > 14.0f)
    {
        timer = 0;
    }
    timer += elapsedTime;

    FrameAttackUpdate(elapsedTime);



    //�A�r���e�B�̃`�F�C�����ɍ��킹�������\��
    for (auto& cAbility : chainAbilitys)
    {
        cAbility->numbers.SetNumbers(cAbility->chain);
    }
}

void PuzzleFrame::Render()
{
    frameModel->Render();
    keepOutBarModel->Render();
    

    //���̑����̍ő�`�F�C�����̃A�r���e�B�̂ݕ\������
    ChainAbility* chains[4];
    for (char type = 0; type < Block::BlockType::END; type++)
    {
        chains[type] = nullptr;
        int chain{};
        for (auto& cAbility : chainAbilitys)
        {
            if (cAbility->type != type)continue;
            if (!chain)
            {
                chain = cAbility->chain;
                chains[type] = cAbility.get();
            }

            if (chains[type]->chain < cAbility->chain)
            {
                chain = chain = cAbility->chain;
                chains[type] = cAbility.get();
            }
        }
        if (chain > 3)
        {
            GameManager::GetPlayer().SetPower(chain,type);
        }
        else
        {
            GameManager::GetPlayer().SetPower(0, type);
        }
        
    }

    Regal::Graphics::Graphics::Instance().Set2DStates();
    int i{};
    for (int i = 0;i < 4;++i)
    {
        if (chains[i] == nullptr)continue;
        if (chains[i]->chain < 4)continue;
        chains[i]->Render(DirectX::XMFLOAT2(200, 400 + 50.0f * i));
    }
    Regal::Graphics::Graphics::Instance().Set3DStates();
}

void PuzzleFrame::DrawDebug()
{
    if (ImGui::BeginMenu(name.c_str()))
    {
        ImGui::Begin(name.c_str());

        frameModel->DrawDebug();

        if(ImGui::TreeNode("Keep Out"))
        {
            keepOutBarModel->DrawDebug();
            ImGui::TreePop();
        }

        ImGui::End();

        ImGui::EndMenu();
    }
}

void PuzzleFrame::ChainAbilityUpdate(float elapsedTime)
{
    
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
    blocks[0] = GetGridBlock(block->GetGridPos().x + 1,block->GetGridPos().y + 1);  //��
    blocks[2] = GetGridBlock(block->GetGridPos().x + 1,block->GetGridPos().y);      //�E��
    blocks[1] = GetGridBlock(block->GetGridPos().x,block->GetGridPos().y + 1);      //����
    blocks[3] = GetGridBlock(block->GetGridPos().x - 1,block->GetGridPos().y + 1);   //�E
    blocks[4] = GetGridBlock(block->GetGridPos().x + 1,block->GetGridPos().y - 1);  //��
    blocks[5] = GetGridBlock(block->GetGridPos().x,block->GetGridPos().y - 1);      //�E��
    blocks[6] = GetGridBlock(block->GetGridPos().x - 1,block->GetGridPos().y);      //����
    blocks[7] = GetGridBlock(block->GetGridPos().x - 1,block->GetGridPos().y - 1);  //��

    //�ċA��A�A������u���b�N���������Ă���ꍇ�͏�����ς���
    if (block->GetAbility())//���łɃA�r���e�B���t�^����Ă���ꍇ
    {
        for (auto& b : blocks)
        {
            if (b == nullptr)continue;
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

    //��x���ċA���Ă�L�Ȃ�
    for (auto& b : blocks)
    {
        if (b == nullptr)continue;

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

            //�ċA
            CheckChainBlock(b);
        }
    }

    //�e�A�r���e�B�̃`�F�C�����𓯃A�h���X���������u���b�N���ŎZ�o
    for (auto& cAbility : chainAbilitys)
    {   
        std::vector<Block*> blocks;
        BlockManager::Instance().FindSameAbilityBlocks(cAbility.get(), blocks);
        cAbility->chain = static_cast<int>(blocks.size());
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
    if (gridsState[gridPos.y][gridPos.x] != LIMIT)
    {
        gridsState[gridPos.y][gridPos.x] = ON_BLOCK;
    }
    gridsBlock[gridPos.y][gridPos.x] = block;

    return true;
}

bool PuzzleFrame::SetBlockOnGrid(Block* block, int x, int y)
{
    gridsBlock[y][x] = block;

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

Block* PuzzleFrame::GetGridBlock(int gridX, int gridY)
{
    if (gridX < 0 || gridX >= MAX_FRAME_WIDTH)return nullptr;
    if (gridY < 0 || gridY >= MAX_FRAME_HEIGHT)return nullptr;

    if (gridsState[gridY][gridX] != ON_BLOCK) return nullptr;

    return gridsBlock[gridY][gridX];
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
            if (gridsBlock[y][x])
            {
                gridsBlock[y][x]->Destroy();

                //�u���b�N��񃊃Z�b�g
                gridsBlock[y][x] = nullptr;
            }

            //�X�e�[�g��񃊃Z�b�g
            gridsState[y][x] = INIT_GRID_STATES[y][x];
        }
    }

    //�A�r���e�B�z�񃊃Z�b�g
    chainAbilitys.clear();
}

void PuzzleFrame::FrameAttackUpdate(float elapsedTime)
{
    if (!isFrameAttack)return;

    if (frameAttackTimer > frameAttackTime)
    {
        for (int y = 0; y < MAX_FRAME_HEIGHT; y++)
        {
            for (int x = 0; x < MAX_FRAME_WIDTH; x++)
            {
                auto* gridBlock{ gridsBlock[y][x] };
                if (gridBlock == nullptr)continue;
                if (gridBlock->GetIsAssault())continue;
                gridBlock->Assault();
                frameAttackTimer = 0;
                goto loopEnd;
            }
        }

        for (int y = 0; y < MAX_FRAME_HEIGHT; y++)
        {
            for (int x = 0; x < MAX_FRAME_WIDTH; x++)
            {
                auto* gridBlock{ gridsBlock[y][x] };
                if (gridBlock)goto loopEnd;
            }
        }
        isFrameAttack = false;
        Clear();
        return;
    }
    loopEnd:

    frameAttackTimer += elapsedTime;
}

void PuzzleFrame::ActiveFrameAttack()
{
    isFrameAttack = true;

    AudioManager::Instance().Play(AudioManager::ALL_BLOCK_ERASE);

    //�������Ă���A�r���e�B�������Ă��Ȃ��u���b�N��S�Ă���
    for (int y = 0; y < MAX_FRAME_HEIGHT; y++)
    {
        for (int x = 0; x < MAX_FRAME_WIDTH; x++)
        {
            auto* gridBlock{ gridsBlock[y][x] };
            if (!gridBlock)continue;
            if (gridBlock->GetAbility())
            {
                if (gridBlock->GetAbility()->chain > 3)continue;
            }

            gridBlock->SetEffectColor(DirectX::XMFLOAT4(1,1,1,1));
            gridBlock->Destroy();

            //�u���b�N��񃊃Z�b�g
            gridsBlock[y][x] = nullptr;

        }
    }

}

PuzzleFrame::ChainAbility::ChainAbility()
{
    chainSprite = std::make_unique<Regal::Resource::Sprite>(
        Regal::Graphics::Graphics::Instance().GetDevice(),
        L"./Resources/Images/chain.png");
}

void PuzzleFrame::ChainAbility::Render(DirectX::XMFLOAT2 pos)
{
    switch (type)
    {
    case Block::BlockType::RED:
        chainSprite->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
        break;
    case Block::BlockType::CYAN:
        chainSprite->SetColor(0.0f, 1.0f, 1.0f, 1.0f);
        break;
    case Block::BlockType::GREEN:
        chainSprite->SetColor(00.4f, 1.0f, 0.0f, 1.0f);
        break;
    case Block::BlockType::PURPLE:
        chainSprite->SetColor(1.0f, 0.0f, 1.0f, 1.0f);
        break;
    }

    auto& graphics{ Regal::Graphics::Graphics::Instance() };
    chainSprite->GetSpriteTransform().SetPos(pos);
    chainSprite->GetSpriteTransform().SetScale(0.5f);
    chainSprite->Render();

    pos.x -= 30;
    numbers.SetPosition(pos);
    numbers.SetNumbers(chain);
    numbers.Render();
}
