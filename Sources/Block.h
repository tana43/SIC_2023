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

    //�f�t�H���g�����̏ꍇ�����Ō��߂�
    Block(bool onGrid = true,int type = -1);
    ~Block() {}

    void CreateResource()override;
    void Initialize()override;
    void Update(float elpasedTime)override;
    void Render()override;
    void DrawDebug()override;

    //�u���b�N���n�ʂɐݒu���ꂽ���ɌĂ�
    void PutOnGrid();

    //�O���b�h���W���烏�[���h���W�ɕϊ����ATransform�ɓ����
    void ConvertToWorldPos();

    //�������Ɉړ��ł��邩��Ԃ��@�����͈ړ�����
    bool CanMoveDown(int moveDistance);
    bool CanMoveRight(int moveDistance);
    bool CanMoveLeft(int moveDistance);
    bool CanMoveBottomRight(int moveDistance);
    bool CanMoveBottomLeft(int moveDistance);
    //�������Ɉړ�
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

    //�ˌ�
    void Assault();

    const bool GetIsAssault() const { return isAssault; }

private:
    void SpinUpdate(float elapsedTime);

    //void AssaultUpdate(float elapsedTime);
private:
    std::unique_ptr<Regal::Model::StaticModel> model;

    //int fallingSpeed{1};//��������}�X��
    float fallTimer;//�������鎞�Ԍv���p�̃^�C�}�[
    float fallingTime;//�P�񗎉�����̂ɂ����鎞��

    //DirectX::XMFLOAT3 velocity{0,0,0};

    //�u����Ă���
    bool isPlaced{ false };

    GridPosition gridPos;//�}�X�ڂł݂����݂̍��W

    //DirectX::XMFLOAT2 blockInterval{DirectX::XMFLOAT2(2.5f,4.0f)};

    int type;

    //�}�X�ڏ�ɂ����ē������ǂ����̃t���O
    bool onGrid;

    //�u���b�N����������`�F�C�����ʂ̃A�h���X�i���������Ă�Ƃ��`�F�C�����Ă�Ƃ��j
    PuzzleFrame::ChainAbility* ability;

    //�����ڂ�����]������p�t���O
    bool isSpin;
    float spinTimer;

    std::unique_ptr<PopEffect> projectilePopEffect;

    //�G�t�F�N�g�Đ��I���܂Ŕj����ҋ@���邽�߂̃t���O
    bool isDestroy{ false };

    bool isAssault{ false };
};

