#include "Block.h"
#include "PuzzleFrame.h"
#include "Easing.h"
#include "BlockManager.h"
#include "EnemyManager.h"
#include "GameManager.h"
#include "AudioManager.h"

DirectX::XMFLOAT2 Block::STARTING_POS = DirectX::XMFLOAT2(5.0f, 2.0f);

Block::Block(bool onGrid,int type) : GameObject("GameObject"),onGrid(onGrid), type(type)
{
	if (type == -1)this->type = rand() % BlockType::END;
}

void Block::CreateResource()
{
    //model = std::make_unique<Regal::Model::StaticModel>("./Resources/Models/LuminousHexagon01.fbx");
	model = std::make_unique<Regal::Model::StaticModel>("./Resources/Models/LuminousCube04.fbx");
	projectilePopEffect = std::make_unique<PopEffect>(100);
}

void Block::Initialize()
{
	model->GetTransform().SetRotationX(DirectX::XMConvertToRadians(45));
	model->GetTransform().SetRotationY(DirectX::XMConvertToRadians(90));
	model->GetTransform().SetScale(DirectX::XMFLOAT3(0.8f,1,1));

	model->SetEmissiveIntensity(1.5f);

	switch (type)
	{
	case BlockType::RED:
		model->SetEmissiveColor(DirectX::XMFLOAT4(1.0f,0.0f,0.0f,1.0f));
		break;
	case BlockType::CYAN:
		model->SetEmissiveColor(DirectX::XMFLOAT4(0.0f,1.0f,1.0f,1.0f));
		break;
	case BlockType::GREEN:
		model->SetEmissiveColor(DirectX::XMFLOAT4(0.4f,1.0f,0.0f,1.0f));
		break;
	case BlockType::PURPLE:
		model->SetEmissiveColor(DirectX::XMFLOAT4(1.0f,0.0f,1.0f,1.0f));
		break;
	}

	projectilePopEffect->SetColor(model->GetEmissiveColor());
}

void Block::Update(float elapsedTime)
{
	if (!isDestroy && !isAssault)
	{
		if (isPlaced)//接地している場合
		{
			//チェイン発動中なら発光を強くする
			if (ability)
			{
				if (ability->chain >= 4)
				{
					model->SetEmissiveIntensity(1.5f + ability->chain * 0.2f);
				}
			}
		}
		if(onGrid)ConvertToWorldPos();

		SpinUpdate(elapsedTime);
	}
	
	if(isDestroy)
	{
		if (!projectilePopEffect->GetIsPlay())
		{
			BlockManager::Instance().Remove(this);
			return;
		}
	}

	//突撃
	if (isAssault)
	{
		if (type == GREEN)
		{
			float spinSpeed{ elapsedTime * 15 };
			model->GetTransform().AddRotation(DirectX::XMFLOAT3(spinSpeed, spinSpeed, 0));

			auto& target{ GameManager::GetPlayer() };
			auto myPos{ model->GetTransform().GetPosition() };
			auto targetPos{ target.GetTransform().GetPosition() };
			DirectX::XMFLOAT3 vec;
			DirectX::XMVECTOR Vec = {
				DirectX::XMVectorSubtract(
				DirectX::XMLoadFloat3(&targetPos),DirectX::XMLoadFloat3(&myPos))
			};
			DirectX::XMVECTOR VecNormal{DirectX::XMVector3Normalize(Vec)};
			DirectX::XMFLOAT3 velocity;
			DirectX::XMStoreFloat3(&velocity, DirectX::XMVectorScale(VecNormal, 50.0f * elapsedTime));

			model->GetTransform().AddPosition(velocity);

			//判定処理
			float length{ DirectX::XMVectorGetX(DirectX::XMVector3Length(Vec)) };

			if (length < 6.0f)
			{
				AudioManager::Instance().Play(AudioManager::BLOCK_ASSAULT);
				target.Heal(1);
				isAssault = false;
				Destroy();
			}
		}
		else
		{
			float spinSpeed{ elapsedTime * 15 };
			model->GetTransform().AddRotation(DirectX::XMFLOAT3(spinSpeed, spinSpeed, 0));

			auto* target{ EnemyManager::Instance().GetEnemy() };
			auto myPos{ model->GetTransform().GetPosition() };
			auto targetPos{ target->GetTransform().GetPosition() };
			DirectX::XMFLOAT3 vec;
			DirectX::XMVECTOR Vec = {
				DirectX::XMVectorSubtract(
				DirectX::XMLoadFloat3(&targetPos),DirectX::XMLoadFloat3(&myPos))
			};
			DirectX::XMVECTOR VecNormal{DirectX::XMVector3Normalize(Vec)};
			DirectX::XMFLOAT3 velocity;
			DirectX::XMStoreFloat3(&velocity, DirectX::XMVectorScale(VecNormal, 50.0f * elapsedTime));

			model->GetTransform().AddPosition(velocity);

			//判定処理
			float length{ DirectX::XMVectorGetX(DirectX::XMVector3Length(Vec)) };

			if (length < 10.0f)
			{
				target->ApplyDamage(GameManager::GetPlayer().GetPower(type));
				AudioManager::Instance().Play(AudioManager::BLOCK_ASSAULT);
				isAssault = false;
				Destroy();
			}
		}
	}

	projectilePopEffect->Update(elapsedTime);
}

void Block::Render()
{
	if (!isDestroy)
	{
		model->Render();
	}

	projectilePopEffect->Render();
}

void Block::DrawDebug()
{
	ImGui::Begin(name.c_str());

	ImGui::SliderInt("GridPosX", &gridPos.x, 0, PuzzleFrame::MAX_FRAME_WIDTH);
	ImGui::SliderInt("GridPosY", &gridPos.y, 0, PuzzleFrame::MAX_FRAME_HEIGHT);

	ImGui::DragFloat("STARTING_POS_X", &STARTING_POS.x,0.1f);
	ImGui::DragFloat("STARTING_POS_Y", &STARTING_POS.y,0.1f);

	projectilePopEffect->DrawDebug();

	ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
	model->DrawDebug();

	ImGui::End();
}

void Block::PutOnGrid()
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
	model->GetTransform().SetPositionX(moveX.x + moveY.x);
	model->GetTransform().SetPositionY(moveX.y + moveY.y);
}

bool Block::CanMoveDown(int moveDistance)
{
	GridPosition moveGridPos{ gridPos.x - moveDistance,gridPos.y - moveDistance };

	if (PuzzleFrame::Instance().MoveBlockDetection(moveGridPos.x, moveGridPos.y))return true;

	return false;
}

bool Block::CanMoveRight(int moveDistance)
{
	GridPosition moveGridPos{ gridPos.x + moveDistance,gridPos.y - moveDistance };

	if (PuzzleFrame::Instance().MoveBlockDetection(moveGridPos.x, moveGridPos.y))return true;

	return false;
}

bool Block::CanMoveLeft(int moveDistance)
{
	GridPosition moveGridPos{ gridPos.x - moveDistance,gridPos.y + moveDistance };

	if (PuzzleFrame::Instance().MoveBlockDetection(moveGridPos.x, moveGridPos.y))return true;

	return false;
}

bool Block::CanMoveBottomRight(int moveDistance)
{
	GridPosition moveGridPos{ gridPos.x,gridPos.y - moveDistance };

	if (PuzzleFrame::Instance().MoveBlockDetection(moveGridPos.x, moveGridPos.y))return true;

	return false;
}

bool Block::CanMoveBottomLeft(int moveDistance)
{
	GridPosition moveGridPos{ gridPos.x - moveDistance,gridPos.y};

	if (PuzzleFrame::Instance().MoveBlockDetection(moveGridPos.x, moveGridPos.y))return true;

	return false;
}

void Block::MoveDown(int moveDistance)
{
	GridPosition moveGridPos{
		gridPos.x - moveDistance,
		gridPos.y - moveDistance
	};
	moveGridPos.x = std::clamp(moveGridPos.x, 0, PuzzleFrame::MAX_FRAME_WIDTH);
	moveGridPos.y = std::clamp(moveGridPos.y, 0, PuzzleFrame::MAX_FRAME_HEIGHT);

	gridPos = moveGridPos;
}

void Block::MoveRight(int moveDistance)
{
	GridPosition moveGridPos{
		gridPos.x + moveDistance,
		gridPos.y - moveDistance
	};
	moveGridPos.x = std::clamp(moveGridPos.x, 0, PuzzleFrame::MAX_FRAME_WIDTH);
	moveGridPos.y = std::clamp(moveGridPos.y, 0, PuzzleFrame::MAX_FRAME_HEIGHT);

	gridPos = moveGridPos;
}

void Block::MoveLeft(int moveDistance)
{
	GridPosition moveGridPos{
		gridPos.x - moveDistance,
		gridPos.y + moveDistance
	};
	moveGridPos.x = std::clamp(moveGridPos.x, 0, PuzzleFrame::MAX_FRAME_WIDTH);
	moveGridPos.y = std::clamp(moveGridPos.y, 0, PuzzleFrame::MAX_FRAME_HEIGHT);

	gridPos = moveGridPos;
}

void Block::MoveBottomRight(int moveDistance)
{
	GridPosition moveGridPos{
		gridPos.x,
		gridPos.y - moveDistance
	};
	moveGridPos.x = std::clamp(moveGridPos.x, 0, PuzzleFrame::MAX_FRAME_WIDTH);
	moveGridPos.y = std::clamp(moveGridPos.y, 0, PuzzleFrame::MAX_FRAME_HEIGHT);

	gridPos = moveGridPos;
}

void Block::MoveBottomLeft(int moveDistance)
{
	GridPosition moveGridPos{
		gridPos.x - moveDistance,
		gridPos.y
	};
	moveGridPos.x = std::clamp(moveGridPos.x, 0, PuzzleFrame::MAX_FRAME_WIDTH);
	moveGridPos.y = std::clamp(moveGridPos.y, 0, PuzzleFrame::MAX_FRAME_HEIGHT);

	gridPos = moveGridPos;
}

void Block::Destroy()
{
	if (isDestroy)return;
	//BlockManager::Instance().Remove(this);
	isDestroy = true;
	projectilePopEffect->Play(GetTransform().GetPosition());
	if(isPlaced)PuzzleFrame::Instance().SetBlockOnGrid(nullptr, gridPos.x, gridPos.y);
}

void Block::Assault()
{
	isAssault = true;
}

void Block::SpinUpdate(float elapsedTime)
{
	if (isSpin)
	{
		GetTransform().SetRotationY(
			Easing::InOutCubic(spinTimer, 0.6f, DirectX::XM_PIDIV2 + DirectX::XM_PI * 2, DirectX::XM_PIDIV2)
		);

		if (spinTimer > 0.6f)
		{
			GetTransform().SetRotationY(DirectX::XM_PIDIV2);
			isSpin = false;
			spinTimer = 0;
		}

		spinTimer += elapsedTime;
	}
}
