#include "EnemyManager.h"
#include <set>
#include "GameManager.h"

EnemyManager::~EnemyManager()
{
	Clear();
}

void EnemyManager::Initialize()
{
	hpGauge = std::make_unique<Regal::Resource::Sprite>(Regal::Graphics::Graphics::Instance().GetDevice(),
		L"./Resources/Images/HpGauge.png","BossHP");
	popEffect = std::make_unique<PopEffect>(300);
	Clear();

	hpGauge->GetSpriteTransform().SetPosition(DirectX::XMFLOAT2(40, 50));
}

void EnemyManager::Update(float elapsedTime)
{
	for (Enemy* enemy : enemys)
	{
		enemy->Update(elapsedTime);
	}

	popEffect->Update(elapsedTime);

	//破棄処理
	//範囲ベースfor文の中で破棄すると不具合が起こる
	for (Enemy* enemy : removes)
	{
		//イテレーターからじゃないと破棄できない
		std::vector<Enemy*>::iterator it =
			std::find(enemys.begin(), enemys.end(), enemy);

		if (it != enemys.end())
		{
			enemys.erase(it);
		}
		//ブロックの破棄処理
		delete enemy;
	}
	//破棄リストクリア
	removes.clear();
}

void EnemyManager::Render()
{
	for (Enemy* enemy : enemys)
	{
		enemy->Render();
	}

	popEffect->Render();

	auto& graphics{ Regal::Graphics::Graphics::Instance() };

	graphics.Set2DStates();

	//hpGauge->GetSpriteTransform().SetPos(spritePos);
	hpGauge->GetSpriteTransform().SetScaleX(1.9f * (static_cast<float>(curEnemy->GetHp()) / static_cast<float>(curEnemy->GetMaxHp())));
	hpGauge->Render();
	graphics.Set3DStates();
}

void EnemyManager::DrawDebug()
{
	if (ImGui::BeginMenu("EnemyManager"))
	{
		if (ImGui::Button("Add"))
		{
			Register(new Enemy);
		}

		//スクロール可能なリスト
		ImGui::BeginChild(ImGui::GetID((void*)0), ImVec2(250, 100), ImGuiWindowFlags_NoTitleBar);
		for (int i = 0; i < enemys.size(); ++i)
		{
			std::string name = "Enemy" + std::to_string(i);
			if (ImGui::BeginMenu(name.c_str()))
			{
				enemys.at(i)->DrawDebug();
				ImGui::EndMenu();
			}
		}
		ImGui::EndChild();

		hpGauge->DrawDebug();

		ImGui::EndMenu();
	}
}

void EnemyManager::Clear()
{
	for (Enemy* enemy : enemys)
	{
		delete enemy;
	}
	enemys.clear();
}

void EnemyManager::Register(Enemy* enemy)
{
	enemys.emplace_back(enemy);
	enemy->CreateResource();
	enemy->Initialize();

	curEnemy = enemy;
}

void EnemyManager::Remove(Enemy* enemy)
{
	removes.emplace_back(enemy);
}

void EnemyManager::Change()
{
	int type{};
	while (true)
	{
		if ((type = rand() % Enemy::END) != curEnemy->GetType())break;
	}

	auto* p = new Enemy(type);
	curEnemy->Remove();
	Register(p);

	GameManager::Instance().AddStageLevel();
}

void EnemyManager::EffectPlay(Enemy* enemy)
{
	popEffect->SetColor(enemy->GetModel()->GetEmissiveColor());
	popEffect->Play(enemy->GetTransform().GetPosition());
}
