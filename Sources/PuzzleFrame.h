#pragma once
#include "RegalLib/Regal.h"
#include "Number.h"

class Block;

using GridState = int;

//�e�O���b�h�ʒu�̃f�[�^�C���[�W
// -1�͉��������Ă��ς�邱�Ƃ̂Ȃ��͈́A�g�̌`�̓s���� �񎟌��z��ŕ\������ɂ͂����Ȃ��Ă��܂�
//  0�̓u���b�N���o���肷��ꏊ�����̂����A�������S�Ė��܂��Ă���ꍇ ���̗�̃u���b�N���폜����
//Ctrl + F�łO���ꂽ�猩���₷��
/*
* -1.-1,-1,-1,-1, 0,-1,-1,-1,
* -1,-1,-1,-1, 0, 0, 0,-1,-1,
* -1,-1,-1, 0, 0, 0, 0, 0,-1,
* -1,-1, 0, 0, 0, 0, 0, 0, 0,
* -1, 0, 0, 0, 0, 0, 0, 0,-1,
*  0, 0, 0, 0, 0, 0, 0,-1,-1,
*  0, 0, 0, 0, 0, 0,-1,-1,-1,
*  0, 0, 0, 0, 0,-1,-1,-1,-1,
*  0, 0, 0, 0,-1,-1,-1,-1,-1,
*/

class PuzzleFrame : public Regal::Game::GameObject
{
private:
    PuzzleFrame() : GameObject("Puzzle Frame") {}
    ~PuzzleFrame() {}

public:

    //���������Ă���ꍇ�A�`�F�C�� + �ǉ����� �Ƃ��ɂ���
    //�`�F�C�����͂S�ȏ�ɂȂ�Ȃ����ʂ𔭊����Ȃ����Q�ȏ�̎��_�Ńu���b�N�ɃA�r���e�B���͕̂t�^�����
    struct ChainAbility
    {
        ChainAbility();

        int chain;//�`�F�C����
        int basePower;//���ʂɉ������U����
        int lastPower;//�������ɗ^����U����

        char type;

        void Render(DirectX::XMFLOAT2 pos);

        Numbers numbers;
        std::unique_ptr<Regal::Resource::Sprite> chainSprite;
    };

    //�g�̑傫��
    static const int MAX_FRAME_WIDTH = 22;
    static const int MAX_FRAME_HEIGHT = 22;

    //�ŉ��i�ɓ���u���b�N���͕Б��łW��
    //�g�ɓ���u���b�N�̋��e�ł���͈́i�ǂ��炩���z�������_�őS�ău���b�N���N���A����j
    static const int FRAME_LIMIT_WIDTH = 16;
    static const int FRAME_LIMIT_HEIGHT = 16;

    //�g���̏������
    static inline const int INIT_GRID_STATES[MAX_FRAME_WIDTH][MAX_FRAME_HEIGHT] =
    {   //1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},//1
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1},//2
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1,-1,-1,-1,-1,-1,-1,-1},//3
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1,-1,-1,-1,-1,-1,-1},//4
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1,-1,-1,-1,-1,-1},//5
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1,-1,-1,-1,-1},//6
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1,-1,-1,-1},//7
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2,-1,-1,-1},//8
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2,-1,-1},//9
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2,-1},//10
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 0, 2, 2, 2},//11
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2,-1},//12
        {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2,-1,-1},//13
        {-1,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2,-1,-1,-1},//14
        {-1,-1,-1, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2,-1,-1,-1,-1},//15
        {-1,-1,-1,-1, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2,-1,-1,-1,-1,-1},//16
        {-1,-1,-1,-1,-1, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2,-1,-1,-1,-1,-1,-1},//17
        {-1,-1,-1,-1,-1,-1, 0, 2, 2, 2, 2, 2, 2, 2, 2,-1,-1,-1,-1,-1,-1,-1},//18
        {-1,-1,-1,-1,-1,-1,-1, 2, 2, 2, 2, 2, 2, 2,-1,-1,-1,-1,-1,-1,-1,-1},//19
        {-1,-1,-1,-1,-1,-1,-1,-1, 2, 2, 2, 2, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1},//20
        {-1,-1,-1,-1,-1,-1,-1,-1,-1, 2, 2, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},//21
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}//22
    };

    static const GridState OUT_RANGE     = -1;//�͈͊O
    static const GridState NONE          = 0;//�u���b�N�Ȃ�
    static const GridState ON_BLOCK      = 1;//�u���b�N����
    static const GridState LIMIT         = 2;//�V��z��

    static PuzzleFrame& Instance() 
    {
        static PuzzleFrame instance;
        return instance;
    }

    void CreateResource()override;
    void Initialize()override;
    void Update(float elapsedTime)override;
    void Render()override;
    void DrawDebug()override;

    void ChainAbilityUpdate(float elapsedTime);

    //�ݒu����Ă���e�u���b�N�Ɠ����I�ȃf�[�^�̓���
    void CheckBlocks();

    //�Ώۂ̃u���b�N����`�F�C�����Ă���u���b�N������Ȃ�A�r���e�B���� �u���b�N�ݒu���ɌĂ�
    void CheckChainBlock(Block* block);

    //�u���b�N��ݒu
    bool SetBlockOnGrid(Block* block);

    //�u���b�N���g���z�����ꏊ�ɐݒu����Ă��邩�̔��� �͈͉z���Ȃ�true
    bool IsOverToleranceLimit();
    //�Ƃ��̂Ƃ��̏���
    void OverToleranceLimit();

    //�����̃}�X�ڂɃu���b�N��u�����Ԃ��ǂ�����Ԃ��A������Ȃ�true
    bool SetBlockDetection(int gridX,int gridY);

    //�����̃}�X�ڂɃu���b�N���ʉ߂ł����Ԃ��ǂ�����Ԃ��A�ł���Ȃ�true
    bool MoveBlockDetection(int gridX,int gridY);

    char GetGridState(int gridX, int gridY) 
    {
        if (gridX < 0 || gridX >= MAX_FRAME_WIDTH)return OUT_RANGE;
        if (gridY < 0 || gridY >= MAX_FRAME_WIDTH)return OUT_RANGE;
        return gridsState[gridY][gridX]; 
    }

    Block* GetGridBlock(int gridX, int gridY);

    //�����̃}�X�ڂ��ݒu���ꂽ�u���b�N���܂߂��g�̒�ɂ��Ă��邩��Ԃ�
    //bool IsBlockOnButtom(int gridX,int gridY);

    //�}�X�ڏ��X���W�Ay���W����map�̃L�[�ɑΉ�����int�l�ɕϊ�����
   /* int ConvertToCombindGridXY(int gridX, int gridY)
    {
        if (gridX < 0 || gridX < MAX_FRAME_WIDTH ||
            gridY < 0 || gridY < MAX_FRAME_HEIGHT)
        {
            assert(0, "�͈͊O�̃O���b�h���W�ɃA�N�Z�X����܂���");
        }
        return gridX + MAX_FRAME_WIDTH * gridY;
    }*/

    void Clear();

    void FrameAttackUpdate(float elapsedTime);
    void ActiveFrameAttack();

    std::vector<std::unique_ptr<ChainAbility>>& GetChainAbilitys() { return chainAbilitys; }

private:
    //���̃}�X�ڂ̏�Ԃ��Ǘ�����@�u���b�N���Ȃ��Ȃ�nullptr������
    Block* gridsBlock[MAX_FRAME_HEIGHT][MAX_FRAME_WIDTH];
    char gridsState[MAX_FRAME_HEIGHT][MAX_FRAME_WIDTH];

    std::unique_ptr<Regal::Model::StaticModel> frameModel;
    std::unique_ptr<Regal::Model::StaticModel> keepOutBarModel;

    std::vector<std::unique_ptr<ChainAbility>> chainAbilitys;

    

    //�S�u���b�N�������čU�������邩
    bool isFrameAttack;
};

