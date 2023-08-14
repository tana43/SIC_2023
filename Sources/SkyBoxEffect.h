#pragma once

#include <Effects.h>
#include <vector>
#include <wrl.h>

#include <DirectXMath.h>

class SkyBoxEffect : public DirectX::IEffect, public DirectX::IEffectMatrices
{
public:
    explicit SkyBoxEffect(ID3D11Device* device);

    virtual void Apply(ID3D11DeviceContext* immediateContext) override;

    virtual void GetVertexShaderBytecode(void const** pShaderByteCode,
        size_t* pByteCodeLength)override;

    void SetTexture(ID3D11ShaderResourceView* value);

    void XM_CALLCONV SetWorld(DirectX::FXMMATRIX value) override;
    void XM_CALLCONV SetView(DirectX::FXMMATRIX value) override;
    void XM_CALLCONV SetProjection(DirectX::FXMMATRIX value) override;
    void XM_CALLCONV SetMatrices(DirectX::FXMMATRIX world, DirectX::CXMMATRIX view, DirectX::CXMMATRIX projection)override;

private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
    std::vector<uint8_t> vsBlob;
};

