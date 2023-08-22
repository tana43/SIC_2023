#pragma once
#include <d3d11.h>
#include <directxmath.h>
#include <wrl.h>
#include <vector>

namespace Regal::Resource
{
    class SpriteBatch
    {
    public:
        Microsoft::WRL::ComPtr<ID3D11VertexShader>  vertexShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader>   pixelShader;
        Microsoft::WRL::ComPtr<ID3D11InputLayout>   inputLayout;
        Microsoft::WRL::ComPtr<ID3D11Buffer>        vertexBuffer;

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
        D3D11_TEXTURE2D_DESC texture2dDesc;

        SpriteBatch(ID3D11Device* device, const wchar_t* filename, size_t maxSprites);
        ~SpriteBatch();

        void Render(ID3D11DeviceContext* immediateContext,
            float dx, float dy, float dw, float dh,
            float r, float g, float b, float a,
            float angle/*degree*/
        );

        void Render(ID3D11DeviceContext* immediateContext,
            float dx, float dy, float dw, float dh,
            float r, float g, float b, float a,
            float angle/*degree*/,
            float sx, float sy, float sw, float sh
        );

        void DrawDebug();

        void Begin(ID3D11DeviceContext* immediateContext,
            ID3D11PixelShader* replacedPixelShader, ID3D11ShaderResourceView* replacedShaderResourceView);
        void End(ID3D11DeviceContext* immediateContext);

    private:
        struct Vertex
        {
            DirectX::XMFLOAT3 position;
            DirectX::XMFLOAT4 color;
            DirectX::XMFLOAT2 texcoord;
        };

        const size_t maxVertices;
        std::vector<Vertex> vertices;
    };
}
