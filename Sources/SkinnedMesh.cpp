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

        const int polygonCount{ fbxMesh->GetPolygonCount() };
        mesh.vertices.resize(polygonCount * 3LL);
        mesh.indices.resize(polygonCount * 3LL);

        FbxStringList uvNames;
        fbxMesh->GetUVSetNames(uvNames);
        const FbxVector4* controlPoints{ fbxMesh->GetControlPoints() };
        for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
        {
            for (int positionInPolygon = 0; positionInPolygon < 3; ++positionInPolygon)
            {
                const int vertexIndex{ polygonIndex * 3 + positionInPolygon };

                Vertex vertex;
                const int polygonVertex{ fbxMesh->GetPolygonVertex(polygonIndex,positionInPolygon) };
                vertex.position.x = static_cast<float>(controlPoints[polygonVertex][0]);
                vertex.position.y = static_cast<float>(controlPoints[polygonVertex][1]);
                vertex.position.z = static_cast<float>(controlPoints[polygonVertex][2]);

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
                mesh.indices.at(vertexIndex) = vertexIndex;
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
    DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
    DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(angle.x,angle.y,angle.z);
    DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position.x,position.y,position.z);

    DirectX::XMFLOAT4X4 world;
    DirectX::XMStoreFloat4x4(&world,S * R * T);

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

        immediateContext->PSSetShaderResources(0, 1, materials.cbegin()->second.shaderResourceViews[0].GetAddressOf());

        Constants data;
        data.world = world;
        data.materialColor = materialColor;
        immediateContext->UpdateSubresource(constantBuffer.Get(), 0, 0, &data, 0, 0);
        immediateContext->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

        D3D11_BUFFER_DESC bufferDesc;
        mesh.indexBuffer->GetDesc(&bufferDesc);
        immediateContext->DrawIndexed(bufferDesc.ByteWidth / sizeof(uint32_t), 0, 0);
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
