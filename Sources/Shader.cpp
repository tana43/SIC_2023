#include "Shader.h"
#include <sstream>
#include "misc.h"

void Shader::CreateVSFromCso(ID3D11Device* device, const char* csoName, ID3D11VertexShader** vertexShader, ID3D11InputLayout** inputLayout, D3D11_INPUT_ELEMENT_DESC* inputElementDesc, SIZE_T BytecodeLength)
{
    //csoファイルから必要な部分のみ切り抜いて頂点シェーダーを作成する
    HRESULT hr{ S_OK };

    FILE* fp{};
    fopen_s(&fp, csoName, "rb");
    _ASSERT_EXPR_A(fp, "CSO file not found");

    fseek(fp, 0, SEEK_END);
    long csoSize{ ftell(fp) };
    fseek(fp, 0, SEEK_SET);

    std::unique_ptr<unsigned char[]>csoData{ std::make_unique<unsigned char[]>(csoSize) };
    fread(csoData.get(), csoSize, 1, fp);
    fclose(fp);

    hr = device->CreateVertexShader(csoData.get(), csoSize, nullptr, vertexShader);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    if (inputLayout)
    {
        hr = device->CreateInputLayout(inputElementDesc, BytecodeLength,
            csoData.get(), csoSize, inputLayout);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }
}

void Shader::CreatePSFromCso(ID3D11Device* device, const char* csoName, ID3D11PixelShader** pixelShader)
{
    HRESULT hr{ S_OK };

    FILE* fp{};
    fopen_s(&fp, csoName, "rb");
    _ASSERT_EXPR_A(fp, "CSO file not found");

    fseek(fp, 0, SEEK_END);
    long csoSize{ ftell(fp) };
    fseek(fp, 0, SEEK_SET);

    std::unique_ptr<unsigned char[]>csoData{ std::make_unique<unsigned char[]>(csoSize) };
    fread(csoData.get(), csoSize, 1, fp);
    fclose(fp);

    hr = device->CreatePixelShader(csoData.get(), csoSize, nullptr, pixelShader);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void Shader::CreateGSFromCso(ID3D11Device* device, const char* csoName, ID3D11GeometryShader** geometryShader)
{
    FILE* fp = nullptr;
    fopen_s(&fp, csoName, "rb");
    _ASSERT_EXPR_A(fp, "CSO File not found");

    fseek(fp, 0, SEEK_END);
    long csoSz = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    std::unique_ptr<unsigned char[]> csoData = std::make_unique<unsigned char[]>(csoSz);
    fread(csoData.get(), csoSz, 1, fp);
    fclose(fp);

    HRESULT hr = device->CreateGeometryShader(csoData.get(), csoSz, nullptr, geometryShader);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void Shader::CreateCSFromCso(ID3D11Device* device, const char* csoName, ID3D11ComputeShader** computeShader)
{
    FILE* fp = nullptr;
    fopen_s(&fp, csoName, "rb");
    _ASSERT_EXPR_A(fp, "CSO File not found");

    fseek(fp, 0, SEEK_END);
    long csoSz = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    std::unique_ptr<unsigned char[]> csoData = std::make_unique<unsigned char[]>(csoSz);
    fread(csoData.get(), csoSz, 1, fp);
    fclose(fp);

    HRESULT hr = device->CreateComputeShader(csoData.get(), csoSz, nullptr, computeShader);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}
