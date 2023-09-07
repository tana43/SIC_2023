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

	//破棄処理
	//範囲ベースfor文の中で破棄すると不具合が起こる
	for (Hexagon* block : removes)
	{
		//イテレーターからじゃないと破棄できない
		std::vector<Hexagon*>::iterator it =
			std::find(blocks.begin(), blocks.end(), block);

		if (it != blocks.end())
		{
			blocks.erase(it);
		}
		//弾丸の破棄処理
		delete block;
	}
	//破棄リストクリア
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
