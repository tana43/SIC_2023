#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <vector>
#include <string>
#include <fbxsdk.h>

struct Scene
{
    struct Node
    {
        uint64_t uniqueId{ 0 };
        std::string name;
        FbxNodeAttribute::EType attribute{FbxNodeAttribute::EType::eUnknown};
        int64_t parentIndex{ -1 };
    };
    std::vector<Node> nodes;
    int64_t indexOf(uint64_t uniqueId)const
    {
        int64_t index{ 0 };
        for (const Node& node : nodes)
        {
            if (node.uniqueId == uniqueId)
            {
                return index;
            }
            ++index;
        }
        return -1;
    }
};

class SkinnedMesh
{
public:
    struct Vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 normal{0, 1, 0};
        DirectX::XMFLOAT2 texcoord{0, 0};
    };
    struct Constants
    {
        DirectX::XMFLOAT4X4 world;
        DirectX::XMFLOAT4 materialColor;
    };

private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

public:
    SkinnedMesh(ID3D11Device* device, const char* fbxFilename, bool triangulate = false);
    virtual ~SkinnedMesh() = default;

protected:
    Scene sceneView;
};

