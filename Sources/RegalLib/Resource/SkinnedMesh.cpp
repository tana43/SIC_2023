#include "SkinnedMesh.h"
#include "../Other/Misc.h"
#include "Shader.h"
#include "Texture.h"

#include <sstream>
#include <fstream>
#include <functional>
#include <filesystem>

#ifdef USE_IMGUI
#include "../../../External/imgui/imgui.h"
#endif // USE_IMGUI

namespace Regal::Resource
{
    int SkinnedMesh::num{};

    struct BoneInfluence
    {
        uint32_t boneIndex;
        float boneWeight;
    };
    using BoneInfluencePerControlPoint = std::vector<BoneInfluence>;

    //ボーン影響度を取得する関数
    void FetchBoneInfluences(const FbxMesh* fbxMesh, std::vector<BoneInfluencePerControlPoint>& boneInfluences)
    {
        const int controlPointsCount{ fbxMesh->GetControlPointsCount() };
        boneInfluences.resize(controlPointsCount);

        const int skinCount{ fbxMesh->GetDeformerCount(FbxDeformer::eSkin) };
        for (int skinIndex = 0; skinIndex < skinCount; ++skinIndex)
        {
            const FbxSkin* fbxSkin{ static_cast<FbxSkin*>(fbxMesh->GetDeformer(skinIndex,FbxDeformer::eSkin)) };

            const int clusterCount{ fbxSkin->GetClusterCount() };
            for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
            {
                const FbxCluster* fbxCluster{ fbxSkin->GetCluster(clusterIndex) };

                const int controlPointIndicesCount{ fbxCluster->GetControlPointIndicesCount() };
                for (int controlPointIndicesIndex = 0;
                    controlPointIndicesIndex < controlPointIndicesCount; ++controlPointIndicesIndex)
                {
                    int controlPointIndex{ fbxCluster->GetControlPointIndices()[controlPointIndicesIndex] };
                    double controlPointWeight{ fbxCluster->GetControlPointWeights()[controlPointIndicesIndex] };
                    BoneInfluence& boneInfluenc{ boneInfluences.at(controlPointIndex).emplace_back() };
                    boneInfluenc.boneIndex = static_cast<uint32_t>(clusterIndex);
                    boneInfluenc.boneWeight = static_cast<float>(controlPointWeight);
                }
            }
        }
    }

    SkinnedMesh::SkinnedMesh(ID3D11Device* device, const char* fbxFilename, bool triangulate, float samplingRate) : myNum(num++)
    {
        //シリアライズされたデータがある場合そちらからロード、なければ従来通りFBXからロード
        std::filesystem::path cerealFilename(fbxFilename);
        cerealFilename.replace_extension("cereal");
        if (std::filesystem::exists(cerealFilename.c_str()))
        {
            std::ifstream ifs(cerealFilename.c_str(), std::ios::binary);
            cereal::BinaryInputArchive deserialization(ifs);
            deserialization(sceneView, meshes, materials, animationClips);
        }
        else
        {
            FbxManager* fbxManager{ FbxManager::Create() };
            FbxScene* fbxScene{ FbxScene::Create(fbxManager,"") };

            FbxImporter* fbxImporter{ FbxImporter::Create(fbxManager,"") };
            bool importStatus{ false };
            importStatus = fbxImporter->Initialize(fbxFilename);
            _ASSERT_EXPR_A(importStatus, fbxImporter->GetStatus().GetErrorString());

            importStatus = fbxImporter->Import(fbxScene);
            _ASSERT_EXPR_A(importStatus, fbxImporter->GetStatus().GetErrorString());

            FbxGeometryConverter fbxConverter(fbxManager);
            if (triangulate)
            {
                fbxConverter.Triangulate(fbxScene, true/*replace*/, false/*legacy*/);
                fbxConverter.RemoveBadPolygonsFromMeshes(fbxScene);
            }

            std::function<void(FbxNode*)> traverse{[&](FbxNode* fbxNode)
                {
                    Scene::Node& node{sceneView.nodes.emplace_back()};
                    node.attribute = fbxNode->GetNodeAttribute() ?
                        fbxNode->GetNodeAttribute()->GetAttributeType() : FbxNodeAttribute::EType::eUnknown;
                    node.name = fbxNode->GetName();
                    node.uniqueId = fbxNode->GetUniqueID();
                    node.parentIndex = sceneView.indexOf(fbxNode->GetParent() ?
                        fbxNode->GetParent()->GetUniqueID() : 0);
                    for (int childIndex = 0; childIndex < fbxNode->GetChildCount(); ++childIndex)
                    {
                        traverse(fbxNode->GetChild(childIndex));
                    }
            } };
            traverse(fbxScene->GetRootNode());

#if 0
            for (const Scene::Node& node : sceneView.nodes)
            {
                FbxNode* fbxNode{ fbxScene->FindNodeByName(node.name.c_str()) };
                //Display node data in the output window as debug
                std::string nodeName = fbxNode->GetName();
                uint64_t uid = fbxNode->GetUniqueID();
                uint64_t parentUid = fbxNode->GetParent() ? fbxNode->GetParent()->GetUniqueID() : 0;
                int32_t type = fbxNode->GetNodeAttribute() ? fbxNode->GetNodeAttribute()->GetAttributeType() : 0;

                std::stringstream debugString;
                debugString << nodeName << ":" << uid << ":" << parentUid << ":" << type << "\n";
                OutputDebugStringA(debugString.str().c_str());
            }
#endif // 1

            FetchMeshes(fbxScene, meshes);

            FetchMaterials(fbxScene, materials);

#if 0
            float samplingRate = 0;
#endif // 0

            FetchAnimations(fbxScene, animationClips, samplingRate);

            fbxManager->Destroy();

            std::ofstream ofs(cerealFilename.c_str(), std::ios::binary);
            cereal::BinaryOutputArchive serialization(ofs);
            serialization(sceneView, meshes, materials, animationClips);
        }



        CreateComObjects(device, fbxFilename);
    }

    inline DirectX::XMFLOAT4X4 ToXMFloat4x4(const FbxAMatrix& fbxamatrix)
    {
        DirectX::XMFLOAT4X4 xmfloat4x4;
        for (int row = 0; row < 4; ++row)
        {
            for (int column = 0; column < 4; ++column)
            {
                xmfloat4x4.m[row][column] = static_cast<float>(fbxamatrix[row][column]);
            }
        }
        return xmfloat4x4;
    }
    inline DirectX::XMFLOAT3 ToXMFloat3(const FbxDouble3& fbxdouble3)
    {
        DirectX::XMFLOAT3 xmfloat3;
        xmfloat3.x = static_cast<float>(fbxdouble3[0]);
        xmfloat3.y = static_cast<float>(fbxdouble3[1]);
        xmfloat3.z = static_cast<float>(fbxdouble3[2]);
        return xmfloat3;
    }
    inline DirectX::XMFLOAT4 ToXMFloat4(const FbxDouble4& fbxdouble4)
    {
        DirectX::XMFLOAT4 xmfloat4;
        xmfloat4.x = static_cast<float>(fbxdouble4[0]);
        xmfloat4.y = static_cast<float>(fbxdouble4[1]);
        xmfloat4.z = static_cast<float>(fbxdouble4[2]);
        xmfloat4.w = static_cast<float>(fbxdouble4[3]);
        return xmfloat4;
    }

    void SkinnedMesh::UpdateAnimation(Animation::Keyframe& keyframe)
    {
        size_t nodeCount{ keyframe.nodes.size() };
        for (size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
        {
            Animation::Keyframe::Node& node{keyframe.nodes.at(nodeIndex)};
            DirectX::XMMATRIX S{DirectX::XMMatrixScaling(node.scaling.x, node.scaling.y, node.scaling.z)};
            DirectX::XMMATRIX R{DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&node.rotation))};
            DirectX::XMMATRIX T{DirectX::XMMatrixTranslation(node.translation.x, node.translation.y, node.translation.z)};

            int64_t parentIndex{ sceneView.nodes.at(nodeIndex).parentIndex };
            DirectX::XMMATRIX P{parentIndex < 0 ? DirectX::XMMatrixIdentity() :
                DirectX::XMLoadFloat4x4(&keyframe.nodes.at(parentIndex).globalTransform)};

            DirectX::XMStoreFloat4x4(&node.globalTransform, S * R * T * P);
        }
    }

    void SkinnedMesh::FetchMeshes(FbxScene* fbxScene, std::vector<Mesh>& meshes)
    {
        for (const Scene::Node& node : sceneView.nodes)
        {
            if (node.attribute != FbxNodeAttribute::EType::eMesh)
            {
                continue;
            }

            FbxNode* fbxNode{ fbxScene->FindNodeByName(node.name.c_str()) };
            FbxMesh* fbxMesh{ fbxNode->GetMesh() };

            Mesh& mesh{ meshes.emplace_back() };

#if 0
            mesh.uniqueId = fbxMesh->GetNode()->GetUniqueID();
            mesh.name = fbxMesh->GetNode()->GetName();
            mesh.nodeIndex = sceneView.indexOf(mesh.uniqueId);
            mesh.defaultGlobalTransform = ToXMFloat4x4(fbxMesh->GetNode()->EvaluateGlobalTransform());
#else
            mesh.uniqueId = node.uniqueId;
            mesh.name = node.name;
            mesh.nodeIndex = sceneView.indexOf(node.uniqueId);
            mesh.defaultGlobalTransform = ToXMFloat4x4(fbxNode->EvaluateGlobalTransform());
#endif // 0



            //ボーン影響度取得
            std::vector<BoneInfluencePerControlPoint> boneInfluences;
            FetchBoneInfluences(fbxMesh, boneInfluences);

            //バインドポーズ取得(初期ポーズ)
            FetchSkeleton(fbxMesh, mesh.bindPose);

            std::vector<Mesh::Subset>& subsets{mesh.subsets};
            const int materialCount{ fbxMesh->GetNode()->GetMaterialCount() };
            subsets.resize(materialCount > 0 ? materialCount : 1);
            for (int materialIndex = 0; materialIndex < materialCount; ++materialIndex)
            {
                const FbxSurfaceMaterial* fbxMaterial{ fbxMesh->GetNode()->GetMaterial(materialIndex) };
                subsets.at(materialIndex).materialName = fbxMaterial->GetName();
                subsets.at(materialIndex).materialUniqueId = fbxMaterial->GetUniqueID();
            }
            if (materialCount > 0)
            {
                const int polygonCount{ fbxMesh->GetPolygonCount() };
                for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
                {
                    const int materialIndex{
                        fbxMesh->GetElementMaterial()->GetIndexArray().GetAt(polygonIndex) };
                    subsets.at(materialIndex).indexCount += 3;
                }
                uint32_t offset{ 0 };
                for (Mesh::Subset& subset : subsets)
                {
                    subset.startIndexLocation = offset;
                    offset += subset.indexCount;
                    subset.indexCount = 0;
                }
            }

            const int polygonCount{ fbxMesh->GetPolygonCount() };
            mesh.vertices.resize(polygonCount * 3LL);
            mesh.indices.resize(polygonCount * 3LL);

            FbxStringList uvNames;
            fbxMesh->GetUVSetNames(uvNames);
            const FbxVector4* controlPoints{ fbxMesh->GetControlPoints() };
            for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
            {
                if (polygonIndex % 1000 == 0)
                {
                    int i = 1;
                }

                const int materialIndex{ materialCount > 0 ?
                fbxMesh->GetElementMaterial()->GetIndexArray().GetAt(polygonIndex) : 0 };
                Mesh::Subset& subset{subsets.at(materialIndex)};
                const uint32_t offset{ subset.startIndexLocation + subset.indexCount };

                for (int positionInPolygon = 0; positionInPolygon < 3; ++positionInPolygon)
                {
                    const int vertexIndex{ polygonIndex * 3 + positionInPolygon };

                    Vertex vertex;
                    const int polygonVertex{ fbxMesh->GetPolygonVertex(polygonIndex,positionInPolygon) };
                    vertex.position.x = static_cast<float>(controlPoints[polygonVertex][0]);
                    vertex.position.y = static_cast<float>(controlPoints[polygonVertex][1]);
                    vertex.position.z = static_cast<float>(controlPoints[polygonVertex][2]);


                    //ウェイト値を頂点にセット
                    const BoneInfluencePerControlPoint& influencePerControlPoint{ boneInfluences.at(polygonVertex) };
                    for (size_t influenceIndex = 0; influenceIndex < influencePerControlPoint.size(); ++influenceIndex)
                    {

                        //const size_t maxBoneInfluence{ influencePerControlPoint.max_size() };
                        if (influenceIndex < MAX_BONE_INFLUENCES)
                        {
                            vertex.boneWeights[influenceIndex] =
                                influencePerControlPoint.at(influenceIndex).boneWeight;
                            vertex.boneIndices[influenceIndex] =
                                influencePerControlPoint.at(influenceIndex).boneIndex;
                        }
                    }

                    if (fbxMesh->GetElementNormalCount() > 0)
                    {
                        FbxVector4 normal;
                        fbxMesh->GetPolygonVertexNormal(polygonIndex, positionInPolygon, normal);
                        vertex.normal.x = static_cast<float>(normal[0]);
                        vertex.normal.y = static_cast<float>(normal[1]);
                        vertex.normal.z = static_cast<float>(normal[2]);
                    }
                    if (fbxMesh->GetElementUVCount() > 0)
                    {
                        FbxVector2 uv;
                        bool unmappedUv;
                        fbxMesh->GetPolygonVertexUV(polygonIndex, positionInPolygon,
                            uvNames[0], uv, unmappedUv);
                        vertex.texcoord.x = static_cast<float>(uv[0]);
                        vertex.texcoord.y = 1.0f - static_cast<float>(uv[1]);
                    }

                    //法線ベクトルの値を取得
                    if (fbxMesh->GenerateTangentsData(0, false))
                    {
                        const FbxGeometryElementTangent* tangent = fbxMesh->GetElementTangent(0);
                        _ASSERT_EXPR(tangent->GetMappingMode() == FbxGeometryElement::EMappingMode::eByPolygonVertex &&
                            tangent->GetReferenceMode() == FbxGeometryElement::EReferenceMode::eDirect,
                            L"Only supports a combination of these modes.");

                        vertex.tangent.x = static_cast<float>(tangent->GetDirectArray().GetAt(vertexIndex)[0]);
                        vertex.tangent.y = static_cast<float>(tangent->GetDirectArray().GetAt(vertexIndex)[1]);
                        vertex.tangent.z = static_cast<float>(tangent->GetDirectArray().GetAt(vertexIndex)[2]);
                        vertex.tangent.w = static_cast<float>(tangent->GetDirectArray().GetAt(vertexIndex)[3]);
                    }

                    mesh.vertices.at(vertexIndex) = std::move(vertex);
                    mesh.indices.at(static_cast<size_t>(offset) + positionInPolygon) = vertexIndex;
                    subset.indexCount++;
                }
            }

            //バウンディングボックス作成
            for (const Vertex& v : mesh.vertices)
            {
                mesh.boundingBox[0].x = std::min<float>(mesh.boundingBox[0].x, v.position.x);
                mesh.boundingBox[0].y = std::min<float>(mesh.boundingBox[0].y, v.position.y);
                mesh.boundingBox[0].z = std::min<float>(mesh.boundingBox[0].z, v.position.z);
                mesh.boundingBox[1].x = std::max<float>(mesh.boundingBox[1].x, v.position.x);
                mesh.boundingBox[1].y = std::max<float>(mesh.boundingBox[1].y, v.position.y);
                mesh.boundingBox[1].z = std::max<float>(mesh.boundingBox[1].z, v.position.z);
            }
        }
    }

    void SkinnedMesh::FetchMaterials(FbxScene* fbxScene, std::unordered_map<uint64_t, Material>& materials)
    {
        const size_t nodeCount{ sceneView.nodes.size() };
        for (size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
        {
            const Scene::Node& node{sceneView.nodes.at(nodeIndex)};
            const FbxNode* fbxNode{ fbxScene->FindNodeByName(node.name.c_str()) };

            const int materialCount{ fbxNode->GetMaterialCount() };
            for (int materialIndex = 0; materialIndex < materialCount; ++materialIndex)
            {
                const FbxSurfaceMaterial* fbxMaterial{ fbxNode->GetMaterial(materialIndex) };

                Material material;
                material.name = fbxMaterial->GetName();
                material.uniqueId = fbxMaterial->GetUniqueID();
                FbxProperty fbxProperty;
                //ディフューズ取得(拡散反射光、ザラつきを表現）
                fbxProperty = fbxMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
                if (fbxProperty.IsValid())
                {
                    const FbxDouble3 color{ fbxProperty.Get<FbxDouble3>() };
                    material.Kd.x = static_cast<float>(color[0]);
                    material.Kd.y = static_cast<float>(color[1]);
                    material.Kd.z = static_cast<float>(color[2]);
                    material.Kd.w = 1.0f;

                    const FbxFileTexture* fbxTexture{ fbxProperty.GetSrcObject<FbxFileTexture>() };
                    material.textureFilenames[0] = fbxTexture ? fbxTexture->GetRelativeFileName() : "";
                }
                //アンビエント取得(環境光、各オブジェクトに別々のパラメータを設定するとキモなるらしい)
                fbxProperty = fbxMaterial->FindProperty(FbxSurfaceMaterial::sAmbient);
                if (fbxProperty.IsValid())
                {
                    const FbxDouble3 color{ fbxProperty.Get<FbxDouble3>() };
                    material.Ka.x = static_cast<float>(color[0]);
                    material.Ka.y = static_cast<float>(color[1]);
                    material.Ka.z = static_cast<float>(color[2]);
                    material.Ka.w = 1.0f;

                    //const FbxFileTexture* fbxTexture{ fbxProperty.GetSrcObject<FbxFileTexture>() };
                    //material.textureFilenames[0] = fbxTexture ? fbxTexture->GetRelativeFileName() : "";
                }
                //スペキュラ取得(鏡面反射光、ツヤを表現する　人工物以外にはあまり使わない方がいい)
                fbxProperty = fbxMaterial->FindProperty(FbxSurfaceMaterial::sSpecular);
                if (fbxProperty.IsValid())
                {
                    const FbxDouble3 color{ fbxProperty.Get<FbxDouble3>() };
                    material.Ks.x = static_cast<float>(color[0]);
                    material.Ks.y = static_cast<float>(color[1]);
                    material.Ks.z = static_cast<float>(color[2]);
                    material.Ks.w = 1.0f;

                    //const FbxFileTexture* fbxTexture{ fbxProperty.GetSrcObject<FbxFileTexture>() };
                    //material.textureFilenames[0] = fbxTexture ? fbxTexture->GetRelativeFileName() : "";
                }

                //法線マップのファイル名を取得
                fbxProperty = fbxMaterial->FindProperty(FbxSurfaceMaterial::sNormalMap);
                if (fbxProperty.IsValid())
                {
                    const FbxFileTexture* fileTexture{ fbxProperty.GetSrcObject<FbxFileTexture>() };
                    material.textureFilenames[1] = fileTexture ? fileTexture->GetRelativeFileName() : "";
                }

                //エミッションテクスチャ取得
                fbxProperty = fbxMaterial->FindProperty(FbxSurfaceMaterial::sEmissive);
                if (fbxProperty.IsValid())
                {
                    const FbxFileTexture* fbxTexture{ fbxProperty.GetSrcObject<FbxFileTexture>() };
                    material.textureFilenames[2] = fbxTexture ? fbxTexture->GetRelativeFileName() : "";
                }
                materials.emplace(material.uniqueId, std::move(material));
            }
            if (materialCount == 0)
            {
                Material dummy{};
                materials.emplace(0, dummy);
            }
        }
    }

    //メッシュからバインドポーズの情報を抽出
    void SkinnedMesh::FetchSkeleton(FbxMesh* fbxMesh, Skeleton& bindPose)
    {
        //メッシュからスキンを取得
        const int deformerCount = fbxMesh->GetDeformerCount(FbxDeformer::eSkin);
        for (int deformerIndex = 0; deformerIndex < deformerCount; ++deformerIndex)
        {
            FbxSkin* skin = static_cast<FbxSkin*>(fbxMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));

            //スキンからクラスター（頂点影響力とそれに対応するボーン）を取得
            const int clusterCount = skin->GetClusterCount();
            bindPose.bones.resize(clusterCount);
            for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
            {
                FbxCluster* cluster = skin->GetCluster(clusterIndex);

                Skeleton::Bone& bone{bindPose.bones.at(clusterIndex)};
                bone.name = cluster->GetLink()->GetName();
                bone.uniqueId = cluster->GetLink()->GetUniqueID();
                bone.parentIndex = bindPose.indexof(cluster->GetLink()->GetParent()->GetUniqueID());
                bone.nodeIndex = sceneView.indexOf(bone.uniqueId);

                //メッシュのローカル空間からシーンのグローバル空間に変換するために使う
                FbxAMatrix referenceGlobalInitPosition;
                cluster->GetTransformMatrix(referenceGlobalInitPosition);

                //ボーンのローカル空間からシーンのグローバル空間に変換するために使う
                FbxAMatrix clusterGlobalInitPosition;
                cluster->GetTransformLinkMatrix(clusterGlobalInitPosition);

                //オフセット行列の作成
                //※メッシュ空間からボーン空間への交換をする行列をオフセット行列と呼ぶ
                bone.offsetTransform
                    = ToXMFloat4x4(clusterGlobalInitPosition.Inverse() * referenceGlobalInitPosition);
            }
        }
    }

    //アニメーション情報抽出
    void SkinnedMesh::FetchAnimations(FbxScene* fbxScene, std::vector<Animation>& animationClips,
        float samplingRate/*１秒間にアニメーションを取り出す回数。この値が０の場合はアニメーションデータはデフォルトのフレームレートでサンプリングされる*/)
    {
        FbxArray<FbxString*> animationStackNames;
        fbxScene->FillAnimStackNameArray(animationStackNames);
        const int animationStackCount{ animationStackNames.GetCount() };
        for (int animationStackIndex = 0; animationStackIndex < animationStackCount; ++animationStackIndex)
        {
            Animation& animationClip{ animationClips.emplace_back() };
            animationClip.name = animationStackNames[animationStackIndex]->Buffer();

            FbxAnimStack* animationStack{ fbxScene->FindMember<FbxAnimStack>(animationClip.name.c_str()) };
            fbxScene->SetCurrentAnimationStack(animationStack);

            //１秒をFbxTimeに変換
            const FbxTime::EMode timeMode{fbxScene->GetGlobalSettings().GetTimeMode()};
            FbxTime oneSecond;
            oneSecond.SetTime(0/*時間*/, 0/*分*/, 1/*秒*/, 0, 0, timeMode);

            //サンプリングレートが０ならデフォルト値を取得
            animationClip.samplingRate = samplingRate > 0 ?
                samplingRate : static_cast<float>(oneSecond.GetFrameRate(timeMode));
            const FbxTime samplingInterval{ static_cast<FbxLongLong>(oneSecond.Get() / animationClip.samplingRate) };

            //アニメーション情報を取得
            const FbxTakeInfo* takeInfo{ fbxScene->GetTakeInfo(animationClip.name.c_str()) };

            const FbxTime startTime{ takeInfo->mLocalTimeSpan.GetStart() };
            const FbxTime stopTime{ takeInfo->mLocalTimeSpan.GetStop() };
            for (FbxTime time = startTime; time < stopTime; time += samplingInterval)
            {
                Animation::Keyframe& keyframe{animationClip.sequence.emplace_back()};

                const size_t nodeCount{ sceneView.nodes.size() };
                keyframe.nodes.resize(nodeCount);
                for (size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
                {
                    FbxNode* fbxNode{ fbxScene->FindNodeByName(sceneView.nodes.at(nodeIndex).name.c_str()) };
                    if (fbxNode)
                    {
                        Animation::Keyframe::Node& node{keyframe.nodes.at(nodeIndex)};

                        //グローバル座標系への交換行列
                        node.globalTransform = ToXMFloat4x4(fbxNode->EvaluateGlobalTransform(time));

                        //parentのローカル座標系に関する変換行列
                        const FbxAMatrix& localTransform{ fbxNode->EvaluateLocalTransform(time) };
                        node.scaling = ToXMFloat3(localTransform.GetS());
                        node.rotation = ToXMFloat4(localTransform.GetQ());
                        node.translation = ToXMFloat3(localTransform.GetT());
                    }
                }
            }
        }
        for (int animationStackIndex = 0; animationStackIndex < animationStackCount; ++animationStackIndex)
        {
            delete animationStackNames[animationStackIndex];
        }
    }

    void SkinnedMesh::BlendAnimations(const Animation::Keyframe* keyframes[2], float factor, Animation::Keyframe& keyframe)
    {
        size_t nodeCount{ keyframes[0]->nodes.size() };
        keyframe.nodes.resize(nodeCount);
        for (size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
        {
            DirectX::XMVECTOR S[2]{
                DirectX::XMLoadFloat3(&keyframes[0]->nodes.at(nodeIndex).scaling),
                DirectX::XMLoadFloat3(&keyframes[1]->nodes.at(nodeIndex).scaling)
            };
            DirectX::XMStoreFloat3(&keyframe.nodes.at(nodeIndex).scaling, DirectX::XMVectorLerp(S[0], S[1], factor));

            DirectX::XMVECTOR R[2]{
                DirectX::XMLoadFloat4(&keyframes[0]->nodes.at(nodeIndex).rotation),
                DirectX::XMLoadFloat4(&keyframes[1]->nodes.at(nodeIndex).rotation)
            };
            DirectX::XMStoreFloat4(&keyframe.nodes.at(nodeIndex).rotation, DirectX::XMQuaternionSlerp(R[0], R[1], factor));

            DirectX::XMVECTOR T[2]{
                DirectX::XMLoadFloat3(&keyframes[0]->nodes.at(nodeIndex).translation),
                DirectX::XMLoadFloat3(&keyframes[1]->nodes.at(nodeIndex).translation)
            };
            DirectX::XMStoreFloat3(&keyframe.nodes.at(nodeIndex).translation, DirectX::XMVectorLerp(T[0], T[1], factor));
        }
    }

    bool SkinnedMesh::AppendAnimations(const char* animationFilename, float samplingRate)
    {
        //シリアライズされたデータがある場合そちらからロード、ねければ従来通りFBXからロード
        std::filesystem::path cerealFilename(animationFilename);
        cerealFilename.replace_extension("cereal");
        if (std::filesystem::exists(cerealFilename.c_str()))
        {
            std::ifstream ifs(cerealFilename.c_str(), std::ios::binary);
            cereal::BinaryInputArchive deserialization(ifs);
            deserialization(animationClips);
        }
        else
        {
            //別ファイルからアニメーション抽出
            FbxManager* fbxManager{ FbxManager::Create() };
            FbxScene* fbxScene{ FbxScene::Create(fbxManager,"") };

            FbxImporter* fbxImporter{ FbxImporter::Create(fbxManager,"") };
            bool importStatus{ false };
            importStatus = fbxImporter->Initialize(animationFilename);
            _ASSERT_EXPR_A(importStatus, fbxImporter->GetStatus().GetErrorString());
            importStatus = fbxImporter->Import(fbxScene);
            _ASSERT_EXPR_A(importStatus, fbxImporter->GetStatus().GetErrorString());

            FetchAnimations(fbxScene, animationClips, samplingRate);

            fbxManager->Destroy();

            std::ofstream ofs(cerealFilename.c_str(), std::ios::binary);
            cereal::BinaryOutputArchive serialization(ofs);
            serialization(animationClips);
        }
        return true;
    }

    void SkinnedMesh::CreateComObjects(ID3D11Device* device, const char* fbxFilename)
    {
        for (Mesh& mesh : meshes)
        {
            HRESULT hr{ S_OK };
            D3D11_BUFFER_DESC bufferDesc{};
            D3D11_SUBRESOURCE_DATA subresourceData{};
            bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * mesh.vertices.size());
            bufferDesc.Usage = D3D11_USAGE_DEFAULT;
            bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bufferDesc.CPUAccessFlags = 0;
            bufferDesc.MiscFlags = 0;
            bufferDesc.StructureByteStride = 0;
            subresourceData.pSysMem = mesh.vertices.data();
            subresourceData.SysMemPitch = 0;
            subresourceData.SysMemSlicePitch = 0;
            hr = device->CreateBuffer(&bufferDesc, &subresourceData,
                mesh.vertexBuffer.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

            bufferDesc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * mesh.indices.size());
            bufferDesc.Usage = D3D11_USAGE_DEFAULT;
            bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            subresourceData.pSysMem = mesh.indices.data();
            hr = device->CreateBuffer(&bufferDesc, &subresourceData,
                mesh.indexBuffer.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

#if 1
            mesh.vertices.clear();
            mesh.indices.clear();
#endif // 1
        }

        //各種テクスチャ読み込み
        for (std::unordered_map<uint64_t, Material>::iterator itr = materials.begin();
            itr != materials.end(); ++itr)
        {
            for (size_t textureIndex = 0; textureIndex < 3; ++textureIndex)
            {
                if (itr->second.textureFilenames[textureIndex].size() > 0)
                {
                    std::filesystem::path path(fbxFilename);
                    path.replace_filename(itr->second.textureFilenames[textureIndex]);
                    D3D11_TEXTURE2D_DESC texture2dDesc;
                    LoadTextureFromFile(device, path.c_str(),
                        itr->second.shaderResourceViews[textureIndex].GetAddressOf(), &texture2dDesc);
                }
                else
                {
                    MakeDummyTexture(device, itr->second.shaderResourceViews[textureIndex].GetAddressOf(),
                        textureIndex == 1 ? 0xFFFF7F7F : 0xFFFFFFFF, 16);
                }
            }
        }

        HRESULT hr = S_OK;
        D3D11_INPUT_ELEMENT_DESC inputElementDesc[]
        {
            {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT},
            {"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT},
            {"TANGENT",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT},
            {"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT},
            {"WEIGHTS",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT},
            {"BONES",0,DXGI_FORMAT_R32G32B32A32_UINT,0,D3D11_APPEND_ALIGNED_ELEMENT},
        };
        Shader::CreateVSFromCso(device, "./Resources/Shader/SkinnedMeshVS.cso", vertexShader.ReleaseAndGetAddressOf(),
            inputLayout.ReleaseAndGetAddressOf(), inputElementDesc, ARRAYSIZE(inputElementDesc));
        Shader::CreatePSFromCso(device, "./Resources/Shader/SkinnedMeshPS.cso", pixelShader.ReleaseAndGetAddressOf());

        D3D11_BUFFER_DESC bufferDesc{};
        bufferDesc.ByteWidth = sizeof(Constants);
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

        hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffer.ReleaseAndGetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }

    void SkinnedMesh::Render(ID3D11DeviceContext* immediateContext, const Animation::Keyframe* keyframe)
    {
        //const DirectX::XMFLOAT4X4 coordinateSystemTransforms[]{
        //    { -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }, // 0:RHS Y-UP
        //    { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }, // 1:LHS Y-UP
        //    { -1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1 }, // 2:RHS Z-UP
        //    { 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 }, // 3:LHS Z-UP
        //};

        //DirectX::XMMATRIX C{DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&coordinateSystemTransforms[0]),
        //    DirectX::XMMatrixScaling(scaleFactor, scaleFactor, scaleFactor))};

        //DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
        //DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(angle.x,angle.y,angle.z);
        //DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position.x,position.y,position.z);

        //DirectX::XMFLOAT4X4 world;
        //DirectX::XMStoreFloat4x4(&world,C * S * R * T);

        auto World{ transform.CalcWorldMatrix() };
        DirectX::XMFLOAT4X4 world;
        DirectX::XMStoreFloat4x4(&world, World);
        Render(immediateContext, world, color, keyframe);
    }

    void SkinnedMesh::Render(
        ID3D11DeviceContext* immediateContext,
        const DirectX::XMFLOAT4X4& world,
        const DirectX::XMFLOAT4& materialColor,
        const Animation::Keyframe* keyframe)
    {
        for (const Mesh& mesh : meshes)
        {
            uint32_t stride{ sizeof(Vertex) };
            uint32_t offset{ 0 };
            immediateContext->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(), &stride, &offset);
            immediateContext->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
            immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            immediateContext->IASetInputLayout(inputLayout.Get());

            immediateContext->VSSetShader(vertexShader.Get(), nullptr, 0);
            immediateContext->PSSetShader(pixelShader.Get(), nullptr, 0);

            //immediateContext->PSSetShaderResources(0, 1, materials.cbegin()->second.shaderResourceViews[0].GetAddressOf());

            Constants data;
            if (keyframe && keyframe->nodes.size() > 0)
            {
                //メッシュのglobalTransformが時間軸で変化しているので、その行列をキーフレームから取得する
                //取得したglobalTransform行列を定数バッファのワールド交換行列に合成する
                const Animation::Keyframe::Node& meshNode{keyframe->nodes.at(mesh.nodeIndex)};
                DirectX::XMStoreFloat4x4(&data.world, DirectX::XMLoadFloat4x4(&meshNode.globalTransform) * DirectX::XMLoadFloat4x4(&world));

                const size_t boneCount{ mesh.bindPose.bones.size() };
                _ASSERT_EXPR(boneCount < MAX_BONES, L"The value of the 'boneCount' has exceeded MAX_BONES.");

                //多分ボーンの情報をローカルからグローバルに変えてる多分多分
                for (size_t boneIndex = 0; boneIndex < boneCount; ++boneIndex)
                {
                    const Skeleton::Bone& bone{mesh.bindPose.bones.at(boneIndex)};
                    const Animation::Keyframe::Node& boneNode{keyframe->nodes.at(bone.nodeIndex)};
                    DirectX::XMStoreFloat4x4(&data.boneTransforms[boneIndex],
                        DirectX::XMLoadFloat4x4(&bone.offsetTransform) *
                        DirectX::XMLoadFloat4x4(&boneNode.globalTransform) *
                        DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&mesh.defaultGlobalTransform))
                    );
                }
            }
            else//アニメーションを持っていない
            {
                DirectX::XMStoreFloat4x4(&data.world,
                    DirectX::XMLoadFloat4x4(&mesh.defaultGlobalTransform) * DirectX::XMLoadFloat4x4(&world));
                for (size_t boneIndex = 0; boneIndex < MAX_BONES; ++boneIndex)
                {
                    data.boneTransforms[boneIndex] = { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
                }
            }
            for (const Mesh::Subset& subset : mesh.subsets)
            {
                const Material& material{ materials.at(subset.materialUniqueId) };
                DirectX::XMStoreFloat4(&data.materialColor,
                    DirectX::XMVectorMultiply(
                        DirectX::XMLoadFloat4(&materialColor),
                        DirectX::XMLoadFloat4(&material.Kd))
                );
                immediateContext->UpdateSubresource(constantBuffer.Get(), 0, 0, &data, 0, 0);
                immediateContext->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

                immediateContext->PSSetShaderResources(0, 1, material.shaderResourceViews[0].GetAddressOf());
                immediateContext->PSSetShaderResources(1, 1, material.shaderResourceViews[1].GetAddressOf());
                //エミッション用テクスチャセット
                immediateContext->PSSetShaderResources(2, 1, material.shaderResourceViews[2].GetAddressOf());

                immediateContext->DrawIndexed(subset.indexCount, subset.startIndexLocation, 0);
            }

            //DirectX::XMStoreFloat4x4(&data.world, DirectX::XMLoadFloat4x4(&mesh.defaultGlobalTransform) * DirectX::XMLoadFloat4x4(&world));
        }
    }

    void SkinnedMesh::_Render(ID3D11DeviceContext* immediateContext, Regal::Game::Transform& transform, const float emissiveIntencity, const DirectX::XMFLOAT4 emissiveColor)
    {
        auto World{ transform.CalcWorldMatrix() };
        DirectX::XMFLOAT4X4 world;
        DirectX::XMStoreFloat4x4(&world, World);
        
        for (const Mesh& mesh : meshes)
        {
            uint32_t stride{ sizeof(Vertex) };
            uint32_t offset{ 0 };
            immediateContext->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(), &stride, &offset);
            immediateContext->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
            immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            immediateContext->IASetInputLayout(inputLayout.Get());

            immediateContext->VSSetShader(vertexShader.Get(), nullptr, 0);
            immediateContext->PSSetShader(pixelShader.Get(), nullptr, 0);

            //immediateContext->PSSetShaderResources(0, 1, materials.cbegin()->second.shaderResourceViews[0].GetAddressOf());

            Constants data;
            data.emissiveIntensity = emissiveIntencity;
            data.emissiveColor = emissiveColor;
            DirectX::XMStoreFloat4x4(&data.world,
                DirectX::XMLoadFloat4x4(&mesh.defaultGlobalTransform) * DirectX::XMLoadFloat4x4(&world));
            for (size_t boneIndex = 0; boneIndex < MAX_BONES; ++boneIndex)
            {
                data.boneTransforms[boneIndex] = { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
            }

            for (const Mesh::Subset& subset : mesh.subsets)
            {
                const Material& material{ materials.at(subset.materialUniqueId) };
                DirectX::XMStoreFloat4(&data.materialColor,
                    DirectX::XMVectorMultiply(
                        DirectX::XMLoadFloat4(&color),
                        DirectX::XMLoadFloat4(&material.Kd))
                );
                immediateContext->UpdateSubresource(constantBuffer.Get(), 0, 0, &data, 0, 0);
                immediateContext->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
                immediateContext->PSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

                immediateContext->PSSetShaderResources(0, 1, material.shaderResourceViews[0].GetAddressOf());
                immediateContext->PSSetShaderResources(1, 1, material.shaderResourceViews[1].GetAddressOf());
                //エミッション用テクスチャセット
                immediateContext->PSSetShaderResources(2, 1, material.shaderResourceViews[2].GetAddressOf());

                immediateContext->DrawIndexed(subset.indexCount, subset.startIndexLocation, 0);
            }
        }
    }

    void SkinnedMesh::DrawDebug()
    {
        ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
        std::string name = "SkinnedMesh " + std::to_string(myNum);
        if (ImGui::TreeNode(name.c_str()))
        {

            ImGui::Checkbox("RenderActive", &renderActive);

            /*ImGui::DragFloat3("Position", &position.x, 0.1f);
            ImGui::DragFloat3("Scale", &scale.x, 0.01f);
            ImGui::DragFloat3("Angle", &angle.x, 0.01f);
            ImGui::DragFloat("ScaleFactor", &scaleFactor, 0.01f);*/

            //transform.DrawDebug();

            ImGui::ColorEdit4("Color", &color.x);
            

            /*if (ImGui::TreeNode("Material"))
            {
                ImGui::DragFloat3("Ka", &materials[0].Ka.x,0.01f);
                ImGui::DragFloat3("Kd", &materials[0].Kd.x,0.01f);
                ImGui::DragFloat3("Ks", &materials[0].Ks.x,0.01f);

                ImGui::TreePop();
            }*/

            ImGui::TreePop();
        }
    }
}