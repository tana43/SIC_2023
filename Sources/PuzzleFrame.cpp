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

    //Transform各項目設定
    auto* transform{ frameModel->GetTransform() };
    //transform->SetRotationY(DirectX::XMConvertToRadians(90));
    transform->SetScaleFactor(0.75f);
    //transform->SetScale(DirectX::XMFLOAT3(0.5f, 1, 0.8f));
    transform->SetPosition(DirectX::XMFLOAT3(2.1f,1.2f,0));

    //カメラ位置を設定
    {
        auto* transform{ frameModel->GetTransform() };
        auto* cTransform{ Regal::Game::Camera::Instance().GetTransform() };

        DirectX::XMFLOAT3 eyePos;
        eyePos = transform->GetPosition();
        eyePos.y += 43.0f;
        eyePos.z -= 88.0f;
        cTransform->SetPosition(eyePos);
    }
}

void PuzzleFrame::Update(float elapsedTime)
{
    //ブロックどっかから順にくるくるさせようぜ

    static float timer;

    for (int y = 0; y < MAX_FRAME_HEIGHT; y++)
    {
        float time = timer - 0.15f * y;
        for (int x = 0; x < MAX_FRAME_WIDTH; x++)
        {
            auto block = GetGridBlock(x,y);
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

    if (timer > 14.0f)
    {
        timer = 0;
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

void PuzzleFrame::ChainAbilityUpdate(float elapsedTime)
{
    
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

void PuzzleFrame::CheckChainBlock(Block* block)
{
    //隣接しているブロックを全て調べる
    Block* blocks[8];
    blocks[0] = GetGridBlock(block->GetGridPos().x + 1,block->GetGridPos().y + 1);  //上
    blocks[2] = GetGridBlock(block->GetGridPos().x + 1,block->GetGridPos().y);      //右上
    blocks[1] = GetGridBlock(block->GetGridPos().x,block->GetGridPos().y + 1);      //左上
    blocks[3] = GetGridBlock(block->GetGridPos().x - 1,block->GetGridPos().y + 1);   //右
    blocks[4] = GetGridBlock(block->GetGridPos().x + 1,block->GetGridPos().y - 1);  //左
    blocks[5] = GetGridBlock(block->GetGridPos().x,block->GetGridPos().y - 1);      //右下
    blocks[6] = GetGridBlock(block->GetGridPos().x - 1,block->GetGridPos().y);      //左下
    blocks[7] = GetGridBlock(block->GetGridPos().x - 1,block->GetGridPos().y - 1);  //下

    //再帰後、連鎖するブロックを検索している場合は処理を変える
    if (block->GetAbility())//すでにアビリティが付与されている場合
    {
        for (auto& b : blocks)
        {
            if (b == nullptr)continue;
            if (block->GetType() != b->GetType())continue;
            if (block->GetAbility() == b->GetAbility())continue;

            if (b->GetAbility()) 
            {
                //アドレスが異なるアビリティを持っているので、どちらかに統合する
                std::vector<Block*> saBlocks;
                BlockManager::Instance().FindSameAbilityBlocks(b->GetAbility(),saBlocks);

                for (auto& saBlock : saBlocks)
                {
                    //アビリティ統合
                    saBlock->SetAbility(block->GetAbility());
                }
            }
        }

        return;
    }

    //一度も再帰していLない
    for (auto& b : blocks)
    {
        if (b == nullptr)continue;

        //タイプが同じではないならcontinue
        if (b->GetType() != block->GetType())continue;

        //隣接しているブロックがすでにチェインしている場合はそれに追加する
        if (b->GetAbility())
        {
            auto cAbility{ b->GetAbility() };
            block->SetAbility(cAbility);
        }
        else//隣接しているブロックがチェインしていない
        {
            auto cAbility{new ChainAbility};
            cAbility->type = block->GetType();

            block->SetAbility(cAbility);
            b->SetAbility(cAbility);
            chainAbilitys.emplace_back(cAbility);

            //再帰
            CheckChainBlock(b);
        }
    }

    //各アビリティのチェイン数を同アドレスを持ったブロック数で算出
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

    //設置できない
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
            //ブロック情報リセット
            gridsBlock[y][x] = nullptr;

            //ステート情報リセット
            gridsState[y][x] = INIT_GRID_STATES[y][x];
        }
    }

    //アビリティ配列リセット
    chainAbilitys.clear();
}
