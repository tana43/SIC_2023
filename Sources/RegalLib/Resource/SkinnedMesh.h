#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <vector>
#include <string>
#include <fbxsdk.h>
#include <unordered_map>

#include <cereal/archives/binary.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/unordered_map.hpp>

#include "../Other/Transform.h"

namespace Regal::Resource
{
    struct Scene
    {
        struct Node
        {
            uint64_t uniqueId{ 0 };
            std::string name;
            FbxNodeAttribute::EType attribute{FbxNodeAttribute::EType::eUnknown};
            int64_t parentIndex{ -1 };

            template<class T>
            void serialize(T& archive)
            {
                archive(uniqueId, name, attribute, parentIndex);
            }
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

        template<class T>
        void serialize(T& archive)
        {
            archive(nodes);
        }
    };

    struct Skeleton
    {
        struct Bone
        {
            uint64_t uniqueId{ 0 };
            std::string name;
            int64_t parentIndex{ -1 };
            int64_t nodeIndex{ 0 };

            DirectX::XMFLOAT4X4 offsetTransform{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

            bool isOrphan() const { return parentIndex < 0; }

            template<class T>
            void serialize(T& archive)
            {
                archive(uniqueId, name, parentIndex, nodeIndex, offsetTransform);;
            }
        };

        std::vector<Bone> bones;
        int64_t indexof(uint64_t uniqueId) const
        {
            int64_t index{ 0 };
            for (const Bone& bone : bones)
            {
                if (bone.uniqueId == uniqueId)
                {
                    return index;
                }
                ++index;
            }
            return -1;
        }

        template<class T>
        void serialize(T& archive)
        {
            archive(bones);
        }
    };

    struct Animation
    {
        std::string name;
        float samplingRate{ 0 };

        struct Keyframe
        {
            struct Node
            {
                //ノードのローカル空間からシーンのグローバル空間に変更するために使う
                DirectX::XMFLOAT4X4 globalTransform{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

                DirectX::XMFLOAT3 scaling{1, 1, 1};
                DirectX::XMFLOAT4 rotation{0, 0, 0, 1};//Rotation Quaternion
                DirectX::XMFLOAT3 translation{0, 0, 0};

                template<class T>
                void serialize(T& archive)
                {
                    archive(globalTransform, scaling, rotation, translation);
                }
            };
            std::vector<Node> nodes;

            template<class T>
            void serialize(T& archive)
            {
                archive(nodes);
            }
        };
        std::vector<Keyframe> sequence;

        template<class T>
        void serialize(T& archive)
        {
            archive(name, samplingRate, sequence);
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
            DirectX::XMFLOAT3 normal;
            DirectX::XMFLOAT4 tangent{1, 0, 0, 1};
            DirectX::XMFLOAT2 texcoord;
            FLOAT boneWeights[MAX_BONE_INFLUENCES]{ 1,0,0,0 };
            INT boneIndices[MAX_BONE_INFLUENCES]{};

            template<class T>
            void serialize(T& archive)
            {
                archive(position, normal, tangent, texcoord, boneWeights, boneIndices);
            }
        };
        static const int MAX_BONES{ 256 };
        struct Constants
        {
            DirectX::XMFLOAT4X4 world;
            DirectX::XMFLOAT4 materialColor;
            DirectX::XMFLOAT4X4 boneTransforms[MAX_BONES]{ {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1} };
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

                template<class T>
                void serialize(T& archive)
                {
                    archive(materialUniqueId, materialName, startIndexLocation, indexCount);
                }
            };
            std::vector<Subset> subsets;

            Skeleton bindPose;

            DirectX::XMFLOAT3 boundingBox[2]
            {
                {+D3D11_FLOAT32_MAX,+D3D11_FLOAT32_MAX,+D3D11_FLOAT32_MAX},
                {-D3D11_FLOAT32_MAX,-D3D11_FLOAT32_MAX,-D3D11_FLOAT32_MAX}
            };

            template<class T>
            void serialize(T& archive)
            {
                archive(uniqueId, name, nodeIndex, subsets, defaultGlobalTransform,
                    bindPose, boundingBox, vertices, indices);
            }

        private:
            Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
            Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
            friend class SkinnedMesh;
        };
        std::vector<Mesh> meshes;

        struct Material
        {
            uint64_t uniqueId{ 0 };
            std::string name;

            DirectX::XMFLOAT4 Ka{0.2f, 0.2f, 0.2f, 1.0f};
            DirectX::XMFLOAT4 Kd{0.8f, 0.8f, 0.8f, 1.0f};
            DirectX::XMFLOAT4 Ks{1.0f, 1.0f, 1.0f, 1.0f};

            std::string textureFilenames[4];
            Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceViews[4];

            template<class T>
            void serialize(T& archive)
            {
                archive(uniqueId, name, Ka, Kd, Ks, textureFilenames);
            }
        };
        std::unordered_map<uint64_t, Material> materials;

        std::vector<Animation> animationClips;

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
        SkinnedMesh(ID3D11Device* device, const char* fbxFilename, bool triangulate = false, float samplingRate = 0);
        virtual ~SkinnedMesh() = default;

        void CreateComObjects(ID3D11Device* device, const char* fbxFilename);

        void Render(ID3D11DeviceContext* immediateContext, const Animation::Keyframe* keyframe);
        void Render(ID3D11DeviceContext* immediateContext,
            const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& materialColor,
            const Animation::Keyframe* keyframe);

        void DrawDebug();

        void UpdateAnimation(Animation::Keyframe& keyframe);

        bool AppendAnimations(const char* animationFilename, float samplingRate);

        void BlendAnimations(const Animation::Keyframe* keyframes[2], float factor, Animation::Keyframe& keyframe);

    protected:
        Scene sceneView;
        int myNum;
        bool renderActive{ true };

        float scaleFactor{ 1.0f };

        void FetchMeshes(FbxScene* fbxScene, std::vector<Mesh>& meshes);
        void FetchMaterials(FbxScene* fbxScene, std::unordered_map<uint64_t, Material>& materials);
        void FetchSkeleton(FbxMesh* fbxMesh, Skeleton& bindPose);
        void FetchAnimations(FbxScene* fbxScene, std::vector<Animation>& animationClips, float samplingRate);

        TransformEuler transform;
    };

    //serializeテンプレート関数
    template<class T>
    void serialize(T& archive, DirectX::XMFLOAT2& v)
    {
        archive(
            cereal::make_nvp("x", v.x),
            cereal::make_nvp("y", v.y)
        );
    }

    template<class T>
    void serialize(T& archive, DirectX::XMFLOAT3& v)
    {
        archive(
            cereal::make_nvp("x", v.x),
            cereal::make_nvp("y", v.y),
            cereal::make_nvp("z", v.z)
        );
    }

    template<class T>
    void serialize(T& archive, DirectX::XMFLOAT4& v)
    {
        archive(
            cereal::make_nvp("x", v.x),
            cereal::make_nvp("y", v.y),
            cereal::make_nvp("z", v.z),
            cereal::make_nvp("w", v.w)
        );
    }

    template<class T>
    void serialize(T& archive, DirectX::XMFLOAT4X4& m)
    {
        archive(
            cereal::make_nvp("_11", m._11), cereal::make_nvp("_12", m._12),
            cereal::make_nvp("_13", m._13), cereal::make_nvp("_14", m._14),
            cereal::make_nvp("_21", m._21), cereal::make_nvp("_22", m._22),
            cereal::make_nvp("_23", m._23), cereal::make_nvp("_24", m._24),
            cereal::make_nvp("_31", m._31), cereal::make_nvp("_32", m._32),
            cereal::make_nvp("_33", m._33), cereal::make_nvp("_34", m._34),
            cereal::make_nvp("_41", m._41), cereal::make_nvp("_42", m._42),
            cereal::make_nvp("_43", m._43), cereal::make_nvp("_44", m._44)
        );
    }
}
