#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <memory>

#ifdef USE_IMGUI
#include "../External/imgui/imgui.h"
#endif

class GeometricPrimitive
{
    static int num;
public:
    struct Vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 normal;
    };
    struct Constants
    {
        DirectX::XMFLOAT4X4 world;
        DirectX::XMFLOAT4 materialColor;
    };

    enum class MeshType
    {
        Cube,
        Sphere,
        Sylinder,
    };

private:
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

public:
    GeometricPrimitive(ID3D11Device* device,MeshType meshType = MeshType::Cube);
    virtual ~GeometricPrimitive() = default;

    void Render(ID3D11DeviceContext* immediateContext);

    void DrawDebug();

private:
    void ShapeCubeMesh(Vertex* vertices, uint32_t* indices);
    void ShapeSphereMesh(Vertex* vertices, uint32_t* indices);
    void ShapeCylinderMesh(Vertex* vertices, uint32_t* indices);

protected:
    int myNum;

    size_t verticesIndex;
    size_t indicesindex;

    void CreateComBuffers(ID3D11Device* device, Vertex* vertices, size_t vertexCount,
        uint32_t* indices, size_t indexCount);

    DirectX::XMFLOAT3 position{0,0,0};
    DirectX::XMFLOAT3 angle{0,0,0};
    DirectX::XMFLOAT3 scale{1,1,1};
    DirectX::XMFLOAT4 color{ 0.5f,0.8f,0.2f,1.0f };
};

