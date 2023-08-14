#include "SkyBoxEffect.h"
#include "ReadData.h"

SkyBoxEffect::SkyBoxEffect(ID3D11Device* device)
{
    vsBlob = DX::ReadData(L"SkyBoxEffectVS.cso");

    auto psBlod = DX::ReadData(L"SkyBoxEffectVS.cso");
}

void SkyBoxEffect::Apply(ID3D11DeviceContext* immediateContext)
{
    immediateContext->PSSetShaderResources(0, 1, texture.GetAddressOf());

    immediateContext->VSSetShader(vertexShader.Get(), nullptr, 0);
    immediateContext->PSSetShader(pixelShader.Get(), nullptr, 0);
}

void SkyBoxEffect::GetVertexShaderBytecode(void const** pShaderByteCode, size_t* pByteCodeLength)
{
    assert(pShaderByteCode != nullptr && pByteCodeLength != nullptr);
    *pShaderByteCode = vsBlob.data();
    *pByteCodeLength = vsBlob.size();
}

void SkyBoxEffect::SetTexture(ID3D11ShaderResourceView* value)
{
    texture = value;
}
