#pragma once

#include <Effects.h>
#include <vector>
#include <wrl.h>

class SkyBoxEffect : public DirectX::IEffect
{
public:
    explicit SkyBoxEffect(ID3D11Device* device);

    virtual void Apply(ID3D11DeviceContext* deviceContext) override;

    virtual void GetVertexShaderBytecode(void const** pShaderByteCode,
        size_t* pByteCodeLength)override;

    void SetTexture(ID3D11ShaderResourceView* value);

private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
    std::vector<uint8_t> vsBlob;
};

