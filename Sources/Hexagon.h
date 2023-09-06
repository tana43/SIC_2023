#pragma once
#include "RegalLib/Regal.h"

class Hexagon : public Regal::Game::GameObject
{
public:
    static DirectX::XMFLOAT2 STARTING_POS;

    struct GridPosition
    {
        int x{};
        int y{};
    };

    Hexagon():GameObject("Hexagon") {}
    ~Hexagon() {}

    void CreateResource()override;
    void Initialize()override;
    void Update(float elpasedTime)override;
    void Render()override;
    void DrawDebug()override;

    //グリッド座標からワールド座標に変換し、Transformに入れる
    void ConvertToWorldPos();

    const Regal::Game::Transform* GetTransform() const { return model->GetTransform(); }

    Regal::Resource::SkinnedMesh* GetSkinnedMesh() { return model->GetSkinnedMesh(); }

    void SetEmissiveColor(const DirectX::XMFLOAT4 color) { model->GetSkinnedMesh()->SetColor(color); }

    void SetEmissiveIntensity(const float intensity) { model->GetSkinnedMesh()->SetEmissiveIntensity(intensity); }

    const GridPosition GetGridPos() const { return gridPos; }
    void SetGridPos(const GridPosition pos) { gridPos = pos; }

    
private:
    std::unique_ptr<Regal::Model::StaticModel> model;

    //int fallingSpeed{1};//落下するマス目
    float fallTimer;//落下する時間計測用のタイマー
    float fallingTime;//１回落下するのにかかる時間

    //DirectX::XMFLOAT3 veloxity{0,0,0};

    bool onGround{ false };

    GridPosition gridPos;//マス目でみた現在の座標

    DirectX::XMFLOAT2 blockInterval{DirectX::XMFLOAT2(2.5f,4.0f)};
};

