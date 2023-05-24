#include "SkinnedMesh.h"
#include "misc.h"
#include "Shader.h"
#include "Texture.h"

#include <sstream>
#include <functional>
#include <filesystem>

#ifdef USE_IMGUI
#include "../imgui/imgui.h"
#endif // USE_IMGUI


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

SkinnedMesh::SkinnedMesh(ID3D11Device* device, const char* fbxFilename, bool triangulate) : myNum(num++)
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

#if 1
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

    traverse(fbxScene->GetRootNode());

    FetchMeshes(fbxScene, meshes);

    FetchMaterials(fbxScene, materials);

    fbxManager->Destroy();

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
        mesh.uniqueId = fbxMesh->GetNode()->GetUniqueID();
        mesh.name = fbxMesh->GetNode()->GetName();
        mesh.nodeIndex = sceneView.indexOf(mesh.uniqueId);
        mesh.defaultGlobalTransform = ToXMFloat4x4(fbxMesh->GetNode()->EvaluateGlobalTransform());

        //ボーン影響度取得
        std::vector<BoneInfluencePerControlPoint> boneInfluences;
        FetchBoneInfluences(fbxMesh, boneInfluences);

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
                for (size_t influenceIndex = 0; influenceIndex < influencePerControlPoint.size();++influenceIndex)
                {
                    const size_t maxBoneInfluence{ influencePerControlPoint.max_size() };
                    if (influenceIndex < maxBoneInfluence)
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

                mesh.vertices.at(vertexIndex) = std::move(vertex);
                mesh.indices.at(static_cast<size_t>(offset) + positionInPolygon) = vertexIndex;
                subset.indexCount++;
            }
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
            materials.emplace(material.uniqueId, std::move(material));
        }
        if (materialCount == 0)
        {
            Material dummy{};
            materials.emplace(0, dummy);
        }
    }
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

    for (std::unordered_map<uint64_t, Material>::iterator itr = materials.begin();
        itr != materials.end(); ++itr)
    {
        if (itr->second.textureFilenames[0].size() > 0)
        {
            std::filesystem::path path(fbxFilename);
            path.replace_filename(itr->second.textureFilenames[0]);
            D3D11_TEXTURE2D_DESC texture2dDesc;
            LoadTextureFromFile(device, path.c_str(),
                itr->second.shaderResourceViews[0].GetAddressOf(), &texture2dDesc);
        }
        else
        {
            MakeDummyTexture(device, itr->second.shaderResourceViews->GetAddressOf(),
                0xFFFFFFFF, 16);
        }
    }

    HRESULT hr = S_OK;
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[]
    {
        {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT},
        {"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT},
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

void SkinnedMesh::Render(ID3D11DeviceContext* immediateContext)
{
    const DirectX::XMFLOAT4X4 coordinateSystemTransforms[]{
        { -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }, // 0:RHS Y-UP
        { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }, // 1:LHS Y-UP
        { -1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1 }, // 2:RHS Z-UP
        { 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 }, // 3:LHS Z-UP
    };

    const float scaleFactor = 1.0f;
    DirectX::XMMATRIX C{DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&coordinateSystemTransforms[2]),
        DirectX::XMMatrixScaling(scaleFactor, scaleFactor, scaleFactor))};

    DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
    DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(angle.x,angle.y,angle.z);
    DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position.x,position.y,position.z);

    DirectX::XMFLOAT4X4 world;
    DirectX::XMStoreFloat4x4(&world,C * S * R * T);

    Render(immediateContext,world,color);
}

void SkinnedMesh::Render(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& materialColor)
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
        DirectX::XMStoreFloat4x4(&data.world,DirectX::XMLoadFloat4x4(&mesh.defaultGlobalTransform) * DirectX::XMLoadFloat4x4(&world));
       /* data.materialColor = materialColor;*/

        for (const Mesh::Subset& subset : mesh.subsets)
        {
            const Material& material{ materials.at(subset.materialUniqueId) };

            DirectX::XMStoreFloat4(&data.materialColor, 
                DirectX::XMVectorMultiply(DirectX::XMLoadFloat4(&materialColor),DirectX::XMLoadFloat4(&material.Kd)));
            immediateContext->UpdateSubresource(constantBuffer.Get(), 0, 0, &data, 0, 0);
            immediateContext->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

            immediateContext->PSSetShaderResources(0, 1, material.shaderResourceViews[0].GetAddressOf());

            immediateContext->DrawIndexed(subset.indexCount, subset.startIndexLocation, 0);
        }
    }
}

void SkinnedMesh::DrawDebug()
{
    std::string name = "SkinnedMesh " + std::to_string(myNum);
    ImGui::Begin(name.c_str());

    ImGui::Checkbox("RenderActive", &renderActive);

    ImGui::DragFloat3("position", &position.x, 0.1f);
    ImGui::DragFloat3("scale", &scale.x, 0.01f);
    ImGui::DragFloat3("angle", &angle.x, 0.01f);
    ImGui::ColorEdit4("color", &color.x);

    ImGui::End();
}
