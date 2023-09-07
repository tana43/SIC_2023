#include "BlocksManager.h"
#include <set>

BlocksManager::~BlocksManager()
{
    Clear();
}

void BlocksManager::Initialize()
{
	Clear();
}

void BlocksManager::Update(float elapsedTime)
{
	for (Hexagon* block : blocks)
	{
		block->Update(elapsedTime);
	}

	//�j������
	//�͈̓x�[�Xfor���̒��Ŕj������ƕs����N����
	for (Hexagon* block : removes)
	{
		//�C�e���[�^�[���炶��Ȃ��Ɣj���ł��Ȃ�
		std::vector<Hexagon*>::iterator it =
			std::find(blocks.begin(), blocks.end(), block);

		if (it != blocks.end())
		{
			blocks.erase(it);
		}
		//�e�ۂ̔j������
		delete block;
	}
	//�j�����X�g�N���A
	removes.clear();
}

void BlocksManager::Render()
{
	for (Hexagon* block : blocks)
	{
		block->Render();
	}
}

void BlocksManager::DrawDebug()
{
	for (Hexagon* block : blocks)
	{
		block->DrawDebug();
	}
}

void BlocksManager::Clear()
{
	for (Hexagon* block : blocks)
	{
		delete block;
	}
}

void BlocksManager::Register(Hexagon* block)
{
	blocks.emplace_back(block);
}

void BlocksManager::Remove(Hexagon* block)
{
	removes.emplace_back(block);
}
