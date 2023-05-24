#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <vector>
#include <string>
#include <fbxsdk.h>
#include <unordered_map>

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
    static int num;

public:
    static const int MAX_BONE_INFLUENCES{ 4 };
    struct Vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 normal{0, 1, 0};
        DirectX::XMFLOAT2 texcoord{0, 0};
        float boneWeights[MAX_BONE_INFLUENCES]{ 1,0,0,0 };
        uint32_t boneIndices[MAX_BONE_INFLUENCES]{};
    };
    struct Constants
    {
        DirectX::XMFLOAT4X4 world;
        DirectX::XMFLOAT4 materialColor;
    };
    struct Mesh
    {
        uint64_t uniqueId{ 0 }; 
        std::string name;
        int64_t nodeIndex{ 0 };

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        DirectX::XMFLOAT4X4 defaultGlobalTransform{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

        struct Subset
        {
            uint64_t materialUniqueId{ 0 };
            std::string materialName;

            uint32_t startIndexLocation{ 0 };
            uint32_t indexCount{ 0 };
        };
        std::vector<Subset> subsets;

    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
        friend class SkinnedMesh;
    };

    struct Material
    {
        uint64_t uniqueId{ 0 };
        std::string name;

        DirectX::XMFLOAT4 Ka{0.2f, 0.2f, 0.2f, 1.0f};
        DirectX::XMFLOAT4 Kd{0.8f, 0.8f, 0.8f, 1.0f};
        DirectX::XMFLOAT4 Ks{1.0f, 1.0f, 1.0f, 1.0f};

        std::string textureFilenames[4];
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceViews[4];
    };

private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

    DirectX::XMFLOAT3 position{ 0, 0, 0 };
    DirectX::XMFLOAT3 angle{ 0, 0, 0 };
    DirectX::XMFLOAT3 scale{ 1, 1, 1 };
    DirectX::XMFLOAT4 color{ 1, 1, 1, 1 };

public:
    SkinnedMesh(ID3D11Device* device, const char* fbxFilename, bool triangulate = false);
    virtual ~SkinnedMesh() = default;

    void FetchMeshes(FbxScene* fbxScene, std::vector<Mesh>& meshes);
    void FetchMaterials(FbxScene* fbxScene,std::unordered_map<uint64_t,Material>& materials);
    void CreateComObjects(ID3D11Device* device, const char* fbxFilename);

    void Render(ID3D11DeviceContext* immediateContext);
    void Render(ID3D11DeviceContext* immediateContext,
        const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& materialColor);

    void DrawDebug();

protected:
    Scene sceneView;
    int myNum;
    bool renderActive{true};

    std::vector<Mesh> meshes;
    std::unordered_map<uint64_t, Material> materials;
};

