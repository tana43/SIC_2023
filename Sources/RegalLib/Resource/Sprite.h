#pragma once
#include <d3d11.h>
#include <directxmath.h>
#include <wrl.h>
#include <sstream>

namespace Regal::Resource
{
    class Sprite
    {
    public:
        Sprite(ID3D11Device* device, const wchar_t* filename);
        ~Sprite();

        void Render(ID3D11DeviceContext* immediateContext,
            float dx, float dy, float dw, float dh, float angle);

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

        void Textout(ID3D11DeviceContext* immediateContext,
            std::string s,
            float x, float y, float w, float h,
            float r, float g, float b, float a
        );

        void DrawDebug();

        void SetColor(float r,float g,float b,float a) { color[0] = r;color[1] = g;color[2] = b;color[3] = a; }

        //シェーダーリソースビュー取得
        const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& GetShaderResourceView() const { return shaderResourceView; }

        //頂点バッファ取得
        const Microsoft::WRL::ComPtr<ID3D11Buffer>& GetVertexBuffer() const { return vertexBuffer; }

        struct Vertex
        {
            DirectX::XMFLOAT3 position;
            DirectX::XMFLOAT4 color;
            DirectX::XMFLOAT2 texcoord;
        };

    private:
        float color[4] = {1,1,1,1};

        Microsoft::WRL::ComPtr<ID3D11VertexShader>  vertexShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader>   pixelShader;
        Microsoft::WRL::ComPtr<ID3D11InputLayout>   inputLayout;
        Microsoft::WRL::ComPtr<ID3D11Buffer>        vertexBuffer;

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
        D3D11_TEXTURE2D_DESC texture2dDesc;
    };
}
