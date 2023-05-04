#pragma once
#include <d3d11.h>

class Shader
{
public:
    static void CreateVSFromCso(ID3D11Device* device, const char* csoName, ID3D11VertexShader** vertexShader,
        ID3D11InputLayout** inputLayout, D3D11_INPUT_ELEMENT_DESC& inputElementDesc, SIZE_T BytecodeLength);
};

