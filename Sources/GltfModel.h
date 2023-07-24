#pragma once
#define NOMINMAX
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "../External/tinygltf-release/tiny_gltf.h"

#include "Transform.h"

class GltfModel
{
    std::string filename;
public:
    GltfModel(ID3D11Device* device, const std::string& filename);
    virtual ~GltfModel() = default;
    struct Scene
    {
        std::string name;
        std::vector<int> nodes;
    };
    std::vector<Scene> scenes;

    struct Node
    {
        std::string name;
        int skin{ -1 };
        int mesh{ -1 };

        std::vector<int> children;

        DirectX::XMFLOAT4 rotation{0, 0, 0, 1};
        DirectX::XMFLOAT3 scale{1, 1, 1};
        DirectX::XMFLOAT3 translation{0, 0, 0};

        DirectX::XMFLOAT4X4 globalTransform{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    };
    std::vector<Node> nodes;

    struct BufferView
    {
        DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
        Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
        size_t strideInBytes{ 0 };
        size_t sizeInBytes{ 0 };
        size_t Count() const
        {
            return sizeInBytes / strideInBytes;
        }
    };
    struct Mesh
    {
        std::string name;
        struct Primitive
        {
            int material;
            std::map<std::string, BufferView> vertexBufferViews;
            BufferView indexBufferView;
        };
        std::vector<Primitive> primitives;
    };
    std::vector<Mesh> meshes;

    
    struct TextureInfo
    {
        int index = -1;
        int texcoord = 0;
    };
    struct NormalTextureInfo
    {
        int index = -1;
        int texcoord = 0;
        float scale = 1;
    };
    struct OcclusionTextureInfo
    {
        int index = -1;
        int texcoord = 0;
        float strength = 1;
    };
    struct PbrMetalicRoughness
    {
        float basecolorFactor[4] = { 1,1,1,1 };
        TextureInfo basecolorTexture;
        float metallicFactor = 1;
        float roughnessFactor = 1;
        TextureInfo metalicRoughnessTexture;
    };
    struct Material
    {
        std::string name;
        struct cbuffer
        {
            float emissiveFactor[3] = { 0,0,0 };
            int alphaMode = 0;
            float alphaCutoff = 0.5f;
            bool doubleSiled = false;

            PbrMetalicRoughness pbrMetallicRoughness;

            NormalTextureInfo normalTexture;
            OcclusionTextureInfo occlusionTexture;
            TextureInfo emissiveTexture;
        };
        cbuffer data;
    };
    std::vector<Material> materials;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> materialResourceView;
    

    void Render(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4& world);
    void Render(ID3D11DeviceContext* immediateContext);

    void DrawDebug();

private:
    void FetchNodes(const tinygltf::Model& gltfModel);
    void CumulateTransforms(std::vector<Node>& nodes);
    BufferView MakeBufferView(const tinygltf::Accessor& accessor);
    void FetchMeshs(ID3D11Device* device, const tinygltf::Model& gltfModel);
    void FetchMaterials(ID3D11Device* device, const tinygltf::Model& gltfModel);

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
    struct PrimitiveConstants
    {
        DirectX::XMFLOAT4X4 world;
        int material{ -1 };
        int hasTangent{ 0 };
        int skin{ -1 };
        int pad;
    };
    Microsoft::WRL::ComPtr<ID3D11Buffer> primitiveCbuffer;

    TransformEuler transform;
};

