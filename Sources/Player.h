#pragma once
#include "RegalLib/Regal.h"
#include "BlockGroup.h"

class Player : public Regal::Game::GameObject
{
public:
    Player() : GameObject("Player") {}
    ~Player() {}

    void CreateResource()override {}
    void Initialize()override;
    void Update(float elapsedTime)override;
    void Render()override {}
    void DrawDebug()override;

    void UseBlocksMove();

    //----------------------------INPUT-------------------------------------
    
    //決定ボタン
    static const bool SelectButton()
    {
        return Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::Enter);
    }

    //左右上下移動
    static const bool MoveUpButton()
    {
        return Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::W);
    }
    static const bool MoveLeftButton()
    {
        return Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::A);
    }
    static const bool MoveDownButton()
    {
        return Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::S);
    }
    static const bool MoveRightButton()
    {
        return Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::D);
    }

    //左右回転
    static const bool RotRightButton()
    {
        return Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::E);
    }
    static const bool RotLeftButton()
    {
        return Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::Q);
    }

    //スペースで盤面クリアもいれよか
    static const bool AllBlockClearButton()
    {
        return Regal::Input::Keyboard::GetKeyDown(DirectX::Keyboard::Space);
    }

    //----------------------------------------------------------------------

    void SetUseBlockGroup(BlockGroup* blockGroup) { useBlockGroup = blockGroup; }
    BlockGroup* GetUseBlockGroup() { return useBlockGroup; }

    void DestroyUseBlock() { delete useBlockGroup; }

private:
    BlockGroup* useBlockGroup;

    int hp;
    int power;//攻撃力
};

