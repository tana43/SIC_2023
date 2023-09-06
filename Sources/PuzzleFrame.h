#pragma once
#include "RegalLib/Regal.h"

using GridState = unsigned int;

class PuzzleFrame : public Regal::Game::GameObject
{
private:
    PuzzleFrame() : GameObject("Puzzle Frame") {}
    ~PuzzleFrame() {}

public:
    //˜g‚Ì‘å‚«‚³
    static const int MAX_FRAME_WIDTH = 10;
    static const int MAX_FRAME_HEIGHT = 15;

    static const GridState NONE     = (1 << 0);
    static const GridState ON_BLOCK = (1 << 1);

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

    void CheckBlocks();

private:
    char frameStates[MAX_FRAME_HEIGHT][MAX_FRAME_WIDTH];
    std::unique_ptr<Regal::Model::StaticModel> frameModel;
};

