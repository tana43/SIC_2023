#include "EnemyManager.h"
#include <set>

EnemyManager::~EnemyManager()
{
	Clear();
}

void EnemyManager::Initialize()
{
	Clear();
}

void EnemyManager::Update(float elapsedTime)
{
	for (Enemy* enemy : enemys)
	{
		enemy->Update(elapsedTime);
	}

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
}

void EnemyManager::Remove(Enemy* enemy)
{
	removes.emplace_back(enemy);
}