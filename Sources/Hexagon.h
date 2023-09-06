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

    //�O���b�h���W���烏�[���h���W�ɕϊ����ATransform�ɓ����
    void ConvertToWorldPos();

    const Regal::Game::Transform* GetTransform() const { return model->GetTransform(); }

    Regal::Resource::SkinnedMesh* GetSkinnedMesh() { return model->GetSkinnedMesh(); }

    void SetEmissiveColor(const DirectX::XMFLOAT4 color) { model->GetSkinnedMesh()->SetColor(color); }

    void SetEmissiveIntensity(const float intensity) { model->GetSkinnedMesh()->SetEmissiveIntensity(intensity); }

    const GridPosition GetGridPos() const { return gridPos; }
    void SetGridPos(const GridPosition pos) { gridPos = pos; }

    
private:
    std::unique_ptr<Regal::Model::StaticModel> model;

    //int fallingSpeed{1};//��������}�X��
    float fallTimer;//�������鎞�Ԍv���p�̃^�C�}�[
    float fallingTime;//�P�񗎉�����̂ɂ����鎞��

    //DirectX::XMFLOAT3 veloxity{0,0,0};

    bool onGround{ false };

    GridPosition gridPos;//�}�X�ڂł݂����݂̍��W

    DirectX::XMFLOAT2 blockInterval{DirectX::XMFLOAT2(2.5f,4.0f)};
};

