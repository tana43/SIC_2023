#include "SkyBoxEffect.h"
#include "ReadData.h"

SkyBoxEffect::SkyBoxEffect(ID3D11Device* device)
{
    vsBlob = DX::ReadData(L"SkyBoxEffectVS.cso");

    auto psBlod = DX::ReadData(L"SkyBoxEffectVS.cso");
}
