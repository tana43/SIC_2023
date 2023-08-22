#include "SpriteBatch.h"
#include "../Other/Misc.h"
#include <sstream>
#include <WICTextureLoader.h>

#ifdef  USE_IMGUI
#include "../../../External/imgui/imgui.h"
#endif //  USE_IMGUI

namespace Regal::Resource
{
    SpriteBatch::SpriteBatch(ID3D11Device* device, const wchar_t* filename, size_t maxSprites)
        :maxVertices(maxSprites * 6)
    {
        /*Vertex vertices[]
        {
            {{-1.0,+1.0,0}, {1,1,1,1},{0,0}},
            {{+1.0,+1.0,0}, {1,1,1,1},{1,0}},
            {{-1.0,-1.0,0}, {1,1,1,1},{0,1}},
            {{+1.0,-1.0,0}, {1,1,1,1},{1,1}},
        };*/

        HRESULT hr{ S_OK };

        std::unique_ptr<Vertex[]> vertices{ std::make_unique<Vertex[]>(maxVertices) };

        //頂点バッファオブジェクト生成
        D3D11_BUFFER_DESC bufferDesc{};
        bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * maxVertices);
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 0;
        D3D11_SUBRESOURCE_DATA subresourceData{};
        subresourceData.pSysMem = vertices.get();
        subresourceData.SysMemPitch = 0;
        subresourceData.SysMemSlicePitch = 0;
        hr = device->CreateBuffer(&bufferDesc, &subresourceData, vertexBuffer.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        //頂点シェーダーオブジェクト生成
        const char* csoName{ "./Resources/Shader/SpriteVS.cso" };

        FILE* fp{};
        fopen_s(&fp, csoName, "rb");
        _ASSERT_EXPR_A(fp, "CSO file not found");

        fseek(fp, 0, SEEK_END);
        long csoSize{ ftell(fp) };
        fseek(fp, 0, SEEK_SET);

        std::unique_ptr<unsigned char[]>csoData{ std::make_unique<unsigned char[]>(csoSize) };
        fread(csoData.get(), csoSize, 1, fp);
        fclose(fp);

        hr = device->CreateVertexShader(csoData.get(), csoSize, nullptr, vertexShader.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        //入力レイアウトオブジェクト生成
        D3D11_INPUT_ELEMENT_DESC inputElementDesc[]
        {
            { "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,   0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
            { "COLOR"   ,0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
            { "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,      0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
        };
        hr = device->CreateInputLayout(inputElementDesc, _countof(inputElementDesc),
            csoData.get(), csoSize, inputLayout.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        //ピクセルシェーダーオブジェクト生成
        csoName = "./Resources/Shader/SpritePS.cso";

        fopen_s(&fp, csoName, "rb");
        _ASSERT_EXPR_A(fp, "CSO file not found");

        fseek(fp, 0, SEEK_END);
        csoSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        csoData = std::make_unique<unsigned char[]>(csoSize);
        fread(csoData.get(), csoSize, 1, fp);
        fclose(fp);

        hr = device->CreatePixelShader(csoData.get(), csoSize, nullptr, pixelShader.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        Microsoft::WRL::ComPtr<ID3D11Resource> resource{};
        hr = DirectX::CreateWICTextureFromFile(
            device, filename, resource.ReleaseAndGetAddressOf(), shaderResourceView.GetAddressOf()
        );
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d{};
        hr = resource->QueryInterface<ID3D11Texture2D>(texture2d.ReleaseAndGetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        texture2d->GetDesc(&texture2dDesc);
    }

    void SpriteBatch::Begin(ID3D11DeviceContext* immediateContext,
        ID3D11PixelShader* replacedPixelShader, ID3D11ShaderResourceView* replacedShaderResourceView)
    {
        //シェーダーとテクスチャの差し替えを可能にしている(UNIT10)
        vertices.clear();
        immediateContext->VSSetShader(vertexShader.Get(), nullptr, 0);
        replacedPixelShader ? immediateContext->PSSetShader(replacedPixelShader, nullptr, 0) : immediateContext->PSSetShader(pixelShader.Get(), nullptr, 0);

        if (replacedShaderResourceView)
        {
            //ここ理解出来てない
            HRESULT hr{ S_OK };
            Microsoft::WRL::ComPtr<ID3D11Resource> resource;
            replacedShaderResourceView->GetResource(resource.GetAddressOf());

            Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
            hr = resource->QueryInterface<ID3D11Texture2D>(texture2d.GetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

            texture2d->GetDesc(&texture2dDesc);

            immediateContext->PSSetShaderResources(0, 1, &replacedShaderResourceView);
        }
        else
        {
            immediateContext->PSSetShaderResources(0, 1, shaderResourceView.GetAddressOf());;
        }
    }

    void SpriteBatch::End(ID3D11DeviceContext* immediateContext)
    {
        HRESULT hr{ S_OK };
        D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
        hr = immediateContext->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        size_t vertexCount = vertices.size();
        _ASSERT_EXPR(maxVertices >= vertexCount, "Buffer overflow");
        Vertex* data{ reinterpret_cast<Vertex*>(mappedSubresource.pData) };
        if (data != nullptr)
        {
            const Vertex* p = vertices.data();
            memcpy_s(data, maxVertices * sizeof(Vertex), p, vertexCount * sizeof(Vertex));
        }
        immediateContext->Unmap(vertexBuffer.Get(), 0);

        UINT stride{ sizeof(Vertex) };
        UINT offset{ 0 };
        immediateContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
        immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        immediateContext->IASetInputLayout(inputLayout.Get());

        immediateContext->Draw(static_cast<UINT>(vertexCount), 0);

        //差し替えたテクスチャを元に戻している(UNIT10)
        Microsoft::WRL::ComPtr<ID3D11Resource> resource;
        shaderResourceView.Get()->GetResource(resource.GetAddressOf());

        Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
        hr = resource->QueryInterface<ID3D11Texture2D>(texture2d.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        texture2d->GetDesc(&texture2dDesc);
    }

    SpriteBatch::~SpriteBatch()
    {
    }

    void SpriteBatch::Render(ID3D11DeviceContext* immediateContext,
        float dx, float dy, float dw, float dh,
        float r, float g, float b, float a,
        float angle/*degree*/
    )
    {
        Render(immediateContext, dx, dy, dw, dh, r, g, b, a, angle, 0, 0, static_cast<float>(texture2dDesc.Width), static_cast<float>(texture2dDesc.Height));
    }

    inline auto rotate(float& x, float& y, float cx, float cy, float angle)
    {
        x -= cx;
        y -= cy;

        float cos{ cosf(DirectX::XMConvertToRadians(angle)) };
        float sin{ sinf(DirectX::XMConvertToRadians(angle)) };
        float tx{ x }, ty{ y };
        x = cos * tx + -sin * ty;
        y = sin * tx + cos * ty;

        x += cx;
        y += cy;
    };
    void SpriteBatch::Render(ID3D11DeviceContext* immediateContext, float dx, float dy, float dw, float dh, float r, float g, float b, float a, float angle, float sx, float sy, float sw, float sh)
    {
        D3D11_VIEWPORT viewport{};
        UINT numViewports{ 1 };
        immediateContext->RSGetViewports(&numViewports, &viewport);



        // (x0,y0)*-----*(x1,y1)
        //         |   /|
        //         |  / |
        //         | /  |
        // (x2,y2)*-----*(x3,y3)
        float x0{ dx };
        float y0{ dy };
        float x1{ dx + dw };
        float y1{ dy };
        float x2{ dx };
        float y2{ dy + dh };
        float x3{ dx + dw };
        float y3{ dy + dh };


        //回転の中心を矩形の中心点にした場合
        float cx = dx + dw * 0.5f;
        float cy = dy + dh * 0.5f;
        rotate(x0, y0, cx, cy, angle);
        rotate(x1, y1, cx, cy, angle);
        rotate(x2, y2, cx, cy, angle);
        rotate(x3, y3, cx, cy, angle);

        //スクリーン座標からNDCへ変換
#define NDC_FROM_SCREEN_X(x) x = 2.0f * x / viewport.Width - 1.0f
#define NDC_FROM_SCREEN_Y(y) y = 1.0f - 2.0f * y / viewport.Height
        NDC_FROM_SCREEN_X(x0); NDC_FROM_SCREEN_Y(y0);
        NDC_FROM_SCREEN_X(x1); NDC_FROM_SCREEN_Y(y1);
        NDC_FROM_SCREEN_X(x2); NDC_FROM_SCREEN_Y(y2);
        NDC_FROM_SCREEN_X(x3); NDC_FROM_SCREEN_Y(y3);

        float u0(sx / texture2dDesc.Width);
        float v0(sy / texture2dDesc.Height);
        float u1{ (sx + sw) / texture2dDesc.Width };
        float v1{ (sy + sh) / texture2dDesc.Height };

        vertices.push_back({ { x0,y0,0 }, { r,g,b,a }, { u0,v0 } });
        vertices.push_back({ { x1,y1,0 }, { r,g,b,a }, { u1,v0 } });
        vertices.push_back({ { x2,y2,0 }, { r,g,b,a }, { u0,v1 } });
        vertices.push_back({ { x2,y2,0 }, { r,g,b,a }, { u0,v1 } });
        vertices.push_back({ { x1,y1,0 }, { r,g,b,a }, { u1,v0 } });
        vertices.push_back({ { x3,y3,0 }, { r,g,b,a }, { u1,v1 } });
    }

    void SpriteBatch::DrawDebug()
    {
#ifdef USE_IMGUI

        ImGui::Begin("Sprite");


        ImGui::End();
#endif
    }
}