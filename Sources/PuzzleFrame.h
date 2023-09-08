#pragma once
#include "RegalLib/Regal.h"
#include "Block.h"

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

    //�g�̑傫��
    static const int MAX_FRAME_WIDTH = 18;
    static const int MAX_FRAME_HEIGHT = 18;

    //�ŉ��i�ɓ���u���b�N���͕Б��łW��
    //�g�ɓ���u���b�N�̋��e�ł���͈́i�ǂ��炩���z�������_�őS�ău���b�N���N���A����j
    static const int FRAME_LIMIT_WIDTH = 16;
    static const int FRAME_LIMIT_HEIGHT = 16;

    //�g���̏������
    static inline const int INIT_GRID_STATES[MAX_FRAME_WIDTH][MAX_FRAME_HEIGHT] =
    {   //1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 2,-1,-1,-1,-1,-1,-1,-1},//1
        {-1,-1,-1,-1,-1,-1,-1,-1,-1, 2, 2, 2,-1,-1,-1,-1,-1,-1},//2
        {-1,-1,-1,-1,-1,-1,-1,-1, 0, 0, 2, 2, 2,-1,-1,-1,-1,-1},//3
        {-1,-1,-1,-1,-1,-1,-1, 0, 0, 0, 0, 2, 2, 2,-1,-1,-1,-1},//4
        {-1,-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 0, 2, 2, 2,-1,-1,-1},//5
        {-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2,-1,-1},//6
        {-1,-1,-1,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2,-1},//7
        {-1,-1,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2},//8
        {-1,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,-1},//9
        {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1,-1},//10
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1,-1,-1},//11
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1,-1,-1,-1},//12
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1,-1,-1,-1,-1},//13
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1,-1,-1,-1,-1,-1},//14
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1,-1,-1,-1,-1,-1,-1},//15
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1,-1,-1,-1,-1,-1,-1,-1},//16
        { 0, 0, 0, 0, 0, 0, 0, 0, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1},//17
        { 0, 0, 0, 0, 0, 0, 0, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}//18
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

    //�ݒu����Ă���e�u���b�N�Ɠ����I�ȃf�[�^�̓���
    void CheckBlocks();

    //�u���b�N��ݒu
    bool SetBlockOnGrid(Block* block);

    //�u���b�N���g���z�����ꏊ�ɐݒu����Ă��邩�̔��� �͈͉z���Ȃ�true
    bool IsOverToleranceLimit();
    //�Ƃ��̂Ƃ��̏���
    void OverToleranceLimit();

    //�����̃}�X�ڂɃu���b�N��u�����Ԃ��ǂ�����Ԃ��A������Ȃ�true
    bool SetBlockDetection(int gridX,int gridY);

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

private:
    //���̃}�X�ڂ̏�Ԃ��Ǘ�����@�u���b�N���Ȃ��Ȃ�nullptr������
    Block* gridsBlock[MAX_FRAME_HEIGHT][MAX_FRAME_WIDTH];
    char gridsState[MAX_FRAME_HEIGHT][MAX_FRAME_WIDTH];

    std::unique_ptr<Regal::Model::StaticModel> frameModel;
};

