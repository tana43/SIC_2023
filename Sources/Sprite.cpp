#include "Sprite.h"
#include "Texture.h"
#include "misc.h"
#include <WICTextureLoader.h>

#ifdef  USE_IMGUI
#include "../External/imgui/imgui.h"
#endif //  USE_IMGUI



Sprite::Sprite(ID3D11Device* device,const wchar_t* filename)
{
    Vertex vertices[]
    {
        {{-1.0,+1.0,0}, {1,1,1,1},{0,0}},
        {{+1.0,+1.0,0}, {1,1,1,1},{1,0}},
        {{-1.0,-1.0,0}, {1,1,1,1},{0,1}},
        {{+1.0,-1.0,0}, {1,1,1,1},{1,1}},
    };

    HRESULT hr{ S_OK };

    //頂点バッファオブジェクト生成
    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.ByteWidth = sizeof(vertices);
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA subresourceData{};
    subresourceData.pSysMem = vertices;
    subresourceData.SysMemPitch = 0;
    subresourceData.SysMemSlicePitch = 0;
    hr = device->CreateBuffer(&bufferDesc, &subresourceData, vertexBuffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    //頂点シェーダーオブジェクト生成
    const char* csoName{ "../Resources/Shader/SpriteVS.cso" };

    FILE* fp{};
    fopen_s(&fp,csoName,"rb");
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
    csoName =  "../Resources/Shader/SpritePS.cso";

    fopen_s(&fp, csoName, "rb");
    _ASSERT_EXPR_A(fp, "CSO file not found");

    fseek(fp, 0, SEEK_END);
    csoSize = ftell(fp) ;
    fseek(fp, 0, SEEK_SET);

    csoData = std::make_unique<unsigned char[]>(csoSize);
    fread(csoData.get(), csoSize, 1, fp);
    fclose(fp);

    hr = device->CreatePixelShader(csoData.get(), csoSize, nullptr, pixelShader.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    hr = LoadTextureFromFile(device, filename, shaderResourceView.GetAddressOf(), &texture2dDesc);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

Sprite::~Sprite()
{
}

void Sprite::Render(ID3D11DeviceContext* immediateContext,
    float dx, float dy, float dw, float dh,
    float r,float g,float b,float a,
    float angle/*degree*/
)
{
    Render(immediateContext, dx, dy, dw, dh, r, g, b, a, angle, 0, 0,static_cast<float>(texture2dDesc.Width), static_cast<float>(texture2dDesc.Height));
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
void Sprite::Render(ID3D11DeviceContext* immediateContext, float dx, float dy, float dw, float dh, float r, float g, float b, float a, float angle, float sx, float sy, float sw, float sh)
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

    //頂点バッファにテクスチャ座標をセット
    HRESULT hr{ S_OK };
    D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
    hr = immediateContext->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    Vertex* vertices{ reinterpret_cast<Vertex*>(mappedSubresource.pData) };
    if (vertices != nullptr)
    {
        vertices[0].position = { x0,y0, 0 };
        vertices[1].position = { x1,y1, 0 };
        vertices[2].position = { x2,y2, 0 };
        vertices[3].position = { x3,y3, 0 };
        vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = { r,g,b,a};

        //テクスチャ座標からUV座標へ変換
        vertices[0].texcoord = { sx / static_cast<float>(texture2dDesc.Width),sy / static_cast<float>(texture2dDesc.Width) };
        vertices[1].texcoord = { (sw + sx) / static_cast<float>(texture2dDesc.Width),sy / static_cast<float>(texture2dDesc.Height) };
        vertices[2].texcoord = { sx / static_cast<float>(texture2dDesc.Width),(sy + sh) / static_cast<float>(texture2dDesc.Height) };
        vertices[3].texcoord = { (sx + sw) / static_cast<float>(texture2dDesc.Width),(sy + sh) / static_cast<float>(texture2dDesc.Height) };
    }
    immediateContext->Unmap(vertexBuffer.Get(), 0);

    //頂点バッファーバインド
    UINT stride{ sizeof(Vertex) };
    UINT offset{ 0 };
    immediateContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);

    //プリミティブタイプおよびデータの順序に関する情報のバインド
    immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    //入力レイアウトオブジェクトのバインド
    immediateContext->IASetInputLayout(inputLayout.Get());

    //シェーダーのバインド
    immediateContext->VSSetShader(vertexShader.Get(), nullptr, 0);
    immediateContext->PSSetShader(pixelShader.Get(), nullptr, 0);

    //シェーダーリソースのバインド
    immediateContext->PSSetShaderResources(0, 1, shaderResourceView.GetAddressOf());

    //プリミティブの描画
    immediateContext->Draw(4, 0);
}

void Sprite::Textout(ID3D11DeviceContext* immediateContext, std::string s, float x, float y, float w, float h, float r, float g, float b, float a)
{
    float sw = static_cast<float>(texture2dDesc.Width / 16);
    float sh = static_cast<float>(texture2dDesc.Height / 16);
    float carriage = 0;
    for (const char c : s)
    {
        Render(immediateContext, x + carriage, y, w, h, r, g, b, a, 0,
            sw * (c & 0x0F), sh * (c >> 4), sw, sh);
        carriage += w;
    }
}

void Sprite::DrawDebug()
{
#ifdef USE_IMGUI

    ImGui::Begin("Sprite");
    

    ImGui::End();
#endif
}
