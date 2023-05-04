#include "GeometricPrimitive.h"
#include "misc.h"
#include <sstream>

GeometricPrimitive::GeometricPrimitive(ID3D11Device* device)
{
    Vertex  vertices[24]{};
    //正立方体のコントロールポイント数は8個、
    //法線の向きが違う頂点が３個あるので頂点情報の総数は8x3 = 24個、
    //頂点情報配列（vertices）にすべて頂点の位置・法線情報を格納する。

    uint32_t indices[36]{};
    //正立方体は６面持ち、１つの面は２つの３角形ポリゴンで構成されるので総数は6x2 = 12個、
    //正立方体を描画するために１２回の３角形ポリゴン描画が必要、よって参照される頂点情報は12x3 = 36回、
    //3角形ポリゴンが参照する頂点情報のインデックス（頂点番号）を描画順に配列（indices）に格納する。
    //時計回りが表面になるように格納すること。

    CreateComBuffers(device, vertices, 24, indices, 36);

    HRESULT hr{ S_OK };

    D3D11_INPUT_ELEMENT_DESC inputElememtDesc[]
    {
        {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
        D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
        D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
    };
    
    //頂点シェーダーオブジェクト生成
    const char* csoName{ "GeometricPrimitive.cso" };

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


    //ピクセルシェーダーオブジェクト生成
    csoName = "SpritePS.cso";

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

    hr = device->CreateVertexShader(csoData.get(), csoSize, nullptr, vertexShader.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    
}

void GeometricPrimitive::CreateComBuffers(ID3D11Device* device, Vertex* vertices, size_t vertexCount, uint32_t* indices, size_t indexCount)
{
}
