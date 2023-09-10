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
        eyePos.y += 30.0f;
        eyePos.z -= 70.0f;
        cTransform->SetPosition(eyePos);
    }
}

void PuzzleFrame::Update(float elapsedTime)
{
    //ブロックどっかから順にくるくるさせようぜ
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
                    Easing::InOutCubic(time - 3.0f, 1.0f/*演出時間*/, DirectX::XM_PIDIV2 + DirectX::XM_PI, DirectX::XM_PIDIV2)
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
    blocks[0] = gridsBlock[block->GetGridPos().x + 1][block->GetGridPos().y + 1];//上
    blocks[2] = gridsBlock[block->GetGridPos().x + 1][block->GetGridPos().y];//右上
    blocks[1] = gridsBlock[block->GetGridPos().x][block->GetGridPos().y + 1];//左上
    blocks[3] = gridsBlock[block->GetGridPos().x - 1][block->GetGridPos().y + 1];//右
    blocks[4] = gridsBlock[block->GetGridPos().x + 1][block->GetGridPos().y - 1];//左
    blocks[5] = gridsBlock[block->GetGridPos().x][block->GetGridPos().y - 1];//右下
    blocks[6] = gridsBlock[block->GetGridPos().x - 1][block->GetGridPos().y];//左下
    blocks[7] = gridsBlock[block->GetGridPos().x - 1][block->GetGridPos().y - 1];//下

    //再帰後、連鎖するブロックを検索している場合は処理を変える
    if (block->GetAbility())//すでにアビリティが付与されている場合
    {
        for (auto& b : blocks)
        {
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

    //一度も再帰していない
    for (auto& b : blocks)
    {
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

            //チェイン数は同アドレスのアビリティを持ったブロックの個数で算出する！！
            //cAbility->chain = 2;

            //再帰
            CheckChainBlock(b);
        }
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
