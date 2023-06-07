#include "Shader.h"
#include <sstream>
#include "misc.h"

void Shader::CreateVSFromCso(ID3D11Device* device, const char* csoName, ID3D11VertexShader** vertexShader, ID3D11InputLayout** inputLayout, D3D11_INPUT_ELEMENT_DESC* inputElementDesc, SIZE_T BytecodeLength)
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
