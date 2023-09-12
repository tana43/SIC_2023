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

	//�j������
	//�͈̓x�[�Xfor���̒��Ŕj������ƕs����N����
	for (Enemy* enemy : removes)
	{
		//�C�e���[�^�[���炶��Ȃ��Ɣj���ł��Ȃ�
		std::vector<Enemy*>::iterator it =
			std::find(enemys.begin(), enemys.end(), enemy);

		if (it != enemys.end())
		{
			enemys.erase(it);
		}
		//�u���b�N�̔j������
		delete enemy;
	}
	//�j�����X�g�N���A
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

		//�X�N���[���\�ȃ��X�g
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