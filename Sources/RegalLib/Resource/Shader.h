#pragma once
#include <d3d11.h>

namespace Regal::Resource
{
    class Shader
    {
    public:
        static void CreateVSFromCso(ID3D11Device* device, const char* csoName, ID3D11VertexShader** vertexShader,
            ID3D11InputLayout** inputLayout, D3D11_INPUT_ELEMENT_DESC* inputElementDesc, SIZE_T BytecodeLength);

        static void CreatePSFromCso(ID3D11Device* device, const char* csoName, ID3D11PixelShader** pixelShader);

        static void CreateGSFromCso(ID3D11Device* device, const char* csoName, ID3D11GeometryShader** geometryShader);

        static void CreateCSFromCso(ID3D11Device* device, const char* csoName, ID3D11ComputeShader** computeShader);

    public:
        //Shader(ID3D11Device* device);
        //~Shader() {}

        //void Initialize();

        ////•`‰æŠJŽn
        //void Begin(ID3D11DeviceContext* immediateContext, const RenderContext& renderContext);
        //void Begin(ID3D11DeviceContext* immediateContext, const RenderContext& renderContext, )
    };
}
