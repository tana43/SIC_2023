#pragma once
#include "RegalLib/Regal.h"
#include "PuzzleFrame.h"
#include "PopEffect.h"

class Block : public Regal::Game::GameObject
{
public:
    static DirectX::XMFLOAT2 STARTING_POS;
    static constexpr float blockInterval = 2.5f;

    struct GridPosition
    {
        int x{};
        int y{};
    };
    
    enum BlockType
    {
        RED,
        CYAN,
        GREEN,
        PURPLE,
        END
    };

    //デフォルト引数の場合乱数で決める
    Block(bool onGrid = true,int type = -1);
    ~Block() {}

    void CreateResource()override;
    void Initialize()override;
    void Update(float elpasedTime)override;
    void Render()override;
    void DrawDebug()override;

    //ブロックが地面に設置された時に呼ぶ
    void PutOnGrid();

    //グリッド座標からワールド座標に変換し、Transformに入れる
    void ConvertToWorldPos();

    //下方向に移動できるかを返す　引数は移動距離
    bool CanMoveDown(int moveDistance);
    bool CanMoveRight(int moveDistance);
    bool CanMoveLeft(int moveDistance);
    bool CanMoveBottomRight(int moveDistance);
    bool CanMoveBottomLeft(int moveDistance);
    //下方向に移動
    void MoveDown(int moveDistance);
    void MoveRight(int moveDistance);
    void MoveLeft(int moveDistance);
    void MoveBottomRight(int moveDistance);
    void MoveBottomLeft(int moveDistance);

    Regal::Game::Transform* GetTransform() const { return model->GetTransform(); }

    Regal::Resource::SkinnedMesh* GetSkinnedMesh() { return model->GetSkinnedMesh(); }

    void SetEmissiveColor(const DirectX::XMFLOAT4 color) { model->GetSkinnedMesh()->SetEmissiveColor(color); }

    void SetEmissiveIntensity(const float intensity) { model->GetSkinnedMesh()->SetEmissiveIntensity(intensity); }

    const GridPosition GetGridPos() const { return gridPos; }
    void SetGridPos(const GridPosition pos) { gridPos = pos; }

    const bool GetIsPlaced() const { return isPlaced; }

    const int GetType() const { return type; }

    void SetOnGrid(const bool og) { onGrid = og; }
    
    void SetAbility(PuzzleFrame::ChainAbility* ca) { ability = ca; }
    PuzzleFrame::ChainAbility* GetAbility() { return ability; }

    void Spin() { isSpin = true; }

    void Destroy();

    const std::unique_ptr<Regal::Model::StaticModel>& GetModel() const { return model; }

    void SetEffectColor(const DirectX::XMFLOAT4 color) { projectilePopEffect->SetColor(color); }

    //突撃
    void Assault();

    const bool GetIsAssault() const { return isAssault; }

private:
    void SpinUpdate(float elapsedTime);

    //void AssaultUpdate(float elapsedTime);
private:
    std::unique_ptr<Regal::Model::StaticModel> model;

    //int fallingSpeed{1};//落下するマス目
    float fallTimer;//落下する時間計測用のタイマー
    float fallingTime;//１回落下するのにかかる時間

    //DirectX::XMFLOAT3 velocity{0,0,0};

    //置かれている
    bool isPlaced{ false };

    GridPosition gridPos;//マス目でみた現在の座標

    //DirectX::XMFLOAT2 blockInterval{DirectX::XMFLOAT2(2.5f,4.0f)};

    int type;

    //マス目上にそって動くかどうかのフラグ
    bool onGrid;

    //ブロックが所属するチェイン効果のアドレス（役が揃ってるとかチェインしてるとか）
    PuzzleFrame::ChainAbility* ability;

    //見た目だけ回転させる用フラグ
    bool isSpin;
    float spinTimer;

    std::unique_ptr<PopEffect> projectilePopEffect;

    //エフェクト再生終了まで破棄を待機するためのフラグ
    bool isDestroy{ false };

    bool isAssault{ false };
};

