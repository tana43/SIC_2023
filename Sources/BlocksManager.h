#pragma once
#include "RegalLib/Game/GameObject.h"
#include <vector>

#include "Hexagon.h"

class BlocksManager
{
private:
    BlocksManager() {}
    ~BlocksManager();

public:
    static BlocksManager& Instance()
    {
        static BlocksManager instance;
        return instance;
    }

    void Initialize();
    void Update(float elapsedTime);
    void Render();
    void DrawDebug();

    void Clear();

    //ê∂ê¨ÇµÇΩÉuÉçÉbÉNÇÃìoò^
    void Register(Hexagon* block);

    void Remove(Hexagon* block);

private:
    std::vector<Hexagon*> blocks;
    std::vector<Hexagon*> removes;
};

