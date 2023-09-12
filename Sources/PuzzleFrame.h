#pragma once
#include "RegalLib/Regal.h"
#include "Number.h"

class Block;

using GridState = int;

//各グリッド位置のデータイメージ
// -1は何があっても変わることのない範囲、枠の形の都合上 二次元配列で表現するにはこうなってしまう
//  0はブロックが出入りする場所ここのｘ軸、ｙ軸が全て埋まっている場合 その列のブロックを削除する
//Ctrl + Fで０入れたら見えやすい
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

    //役が揃っている場合、チェイン + 追加効果 とかにする
    //チェイン数は４つ以上にならない効果を発揮しないが２つ以上の時点でブロックにアビリティ自体は付与される
    struct ChainAbility
    {
        ChainAbility();

        int chain;//チェイン数
        int basePower;//効果に応じた攻撃力
        int lastPower;//消去時に与える攻撃力

        char type;

        void Render(DirectX::XMFLOAT2 pos);

        Numbers numbers;
        std::unique_ptr<Regal::Resource::Sprite> chainSprite;
    };

    //枠の大きさ
    static const int MAX_FRAME_WIDTH = 22;
    static const int MAX_FRAME_HEIGHT = 22;

    //最下段に入るブロック数は片側で８つ
    //枠に入るブロックの許容できる範囲（どちらかを越えた時点で全てブロックをクリアする）
    static const int FRAME_LIMIT_WIDTH = 16;
    static const int FRAME_LIMIT_HEIGHT = 16;

    //枠内の初期状態
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

    static const GridState OUT_RANGE     = -1;//範囲外
    static const GridState NONE          = 0;//ブロックなし
    static const GridState ON_BLOCK      = 1;//ブロックあり
    static const GridState LIMIT         = 2;//天井越え

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

    //設置されている各ブロックと内部的なデータの同期
    void CheckBlocks();

    //対象のブロックからチェインしているブロックがあるならアビリティ生成 ブロック設置時に呼ぶ
    void CheckChainBlock(Block* block);

    //ブロックを設置
    bool SetBlockOnGrid(Block* block);

    //ブロックが枠を越えた場所に設置されているかの判定 範囲越えならtrue
    bool IsOverToleranceLimit();
    //とそのときの処理
    void OverToleranceLimit();

    //引数のマス目にブロックを置ける状態かどうかを返す、おけるならtrue
    bool SetBlockDetection(int gridX,int gridY);

    //引数のマス目にブロックが通過できる状態かどうかを返す、できるならtrue
    bool MoveBlockDetection(int gridX,int gridY);

    char GetGridState(int gridX, int gridY) 
    {
        if (gridX < 0 || gridX >= MAX_FRAME_WIDTH)return OUT_RANGE;
        if (gridY < 0 || gridY >= MAX_FRAME_WIDTH)return OUT_RANGE;
        return gridsState[gridY][gridX]; 
    }

    Block* GetGridBlock(int gridX, int gridY);

    //引数のマス目が設置されたブロックも含めた枠の底についているかを返す
    //bool IsBlockOnButtom(int gridX,int gridY);

    //マス目上のX座標、y座標からmapのキーに対応したint値に変換する
   /* int ConvertToCombindGridXY(int gridX, int gridY)
    {
        if (gridX < 0 || gridX < MAX_FRAME_WIDTH ||
            gridY < 0 || gridY < MAX_FRAME_HEIGHT)
        {
            assert(0, "範囲外のグリッド座標にアクセスされました");
        }
        return gridX + MAX_FRAME_WIDTH * gridY;
    }*/

    void Clear();

    void FrameAttackUpdate(float elapsedTime);
    void ActiveFrameAttack();

    std::vector<std::unique_ptr<ChainAbility>>& GetChainAbilitys() { return chainAbilitys; }

private:
    //そのマス目の状態を管理する　ブロックがないならnullptrを入れる
    Block* gridsBlock[MAX_FRAME_HEIGHT][MAX_FRAME_WIDTH];
    char gridsState[MAX_FRAME_HEIGHT][MAX_FRAME_WIDTH];

    std::unique_ptr<Regal::Model::StaticModel> frameModel;
    std::unique_ptr<Regal::Model::StaticModel> keepOutBarModel;

    std::vector<std::unique_ptr<ChainAbility>> chainAbilitys;

    

    //全ブロックを消して攻撃をするか
    bool isFrameAttack;
};

