#pragma once

#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include "../Resource/Sprite.h"

namespace Regal::Graphics
{
    class SkyBox
    {
    public:
        SkyBox(ID3D11Device* device, Regal::Resource::Sprite* sTitle_0);
        ~SkyBox() {}

        struct  Vertex
        {
            DirectX::XMFLOAT3 position;
        };

        struct  CbScene
        {
            DirectX::XMFLOAT4X4 inverseViewProjection;
            DirectX::XMFLOAT4 viewPosition;
        };

        void Render(ID3D11DeviceContext* immediateContext, DirectX::XMMATRIX V/*view*/, DirectX::XMMATRIX P/*projection*/);

    private:

        Microsoft::WRL::ComPtr<ID3D11Buffer> sceneConstantBuffer;
        Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

        //Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;

        Microsoft::WRL::ComPtr<ID3D11BlendState>			blendState;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState>		rasterizerState;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState>		depthStencilState;
        Microsoft::WRL::ComPtr<ID3D11SamplerState>			samplerState;

        Regal::Resource::Sprite* sTitle_0;
    };
}
