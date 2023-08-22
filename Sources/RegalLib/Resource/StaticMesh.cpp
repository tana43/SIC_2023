#include "StaticMesh.h"

#include <vector>
#include <string>
#include <filesystem>

#include "../Resource/Shader.h"
#include "../Other/Misc.h"
#include "Texture.h"

namespace Regal::Resource
{
    int StaticMesh::num{ 0 };

    StaticMesh::StaticMesh(ID3D11Device* device, const wchar_t* objFilename, bool reverseV, DirectX::XMFLOAT3 pos, DirectX::XMFLOAT4 color)
        : reverseV(reverseV), position(pos), color(color), myNum(num++)
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        uint32_t currentIndex{ 0 };

        std::vector<DirectX::XMFLOAT3> positions;
        std::vector<DirectX::XMFLOAT3> normals;
        std::vector<DirectX::XMFLOAT2> texcoords;
        std::vector<std::wstring> mtlFilenames;

        DirectX::XMFLOAT3 max{};
        DirectX::XMFLOAT3 min{};

        std::wifstream fin(objFilename);
        _ASSERT_EXPR(fin, L"'OBJ file not found.");
        wchar_t command[256];
        while (fin)
        {
            fin >> command;
            if (0 == wcscmp(command, L"v"))
            {
                float x, y, z;
                fin >> x >> y >> z;
                positions.push_back({ x,y,z });
                fin.ignore(1024, L'\n');

                if (x > max.x)max.x = x;
                if (y > max.y)max.y = y;
                if (z > max.z)max.z = z;
                if (x < min.x)min.x = x;
                if (y < min.y)min.y = y;
                if (z < min.z)min.z = z;
            }
            else if (0 == wcscmp(command, L"vn"))
            {
                float i, j, k;
                fin >> i >> j >> k;
                normals.push_back({ i,j,k });
                fin.ignore(1024, L'\n');
            }
            else if (0 == wcscmp(command, L"vt"))
            {
                float u, v;
                fin >> u >> v;
                if (reverseV)texcoords.push_back({ u,1.0f - v });
                else texcoords.push_back({ u,v });
                fin.ignore(1024, L'\n');
            }
            else if (0 == wcscmp(command, L"f"))
            {
                for (size_t i = 0; i < 3; i++)
                {
                    Vertex vertex;
                    size_t v, vt, vn;

                    fin >> v;
                    vertex.position = positions.at(v - 1);
                    if (L'/' == fin.peek())
                    {
                        fin.ignore(1);
                        if (L'/' != fin.peek())
                        {
                            fin >> vt;
                            vertex.texcoord = texcoords.at(vt - 1);
                        }
                        if (L'/' == fin.peek())
                        {
                            fin.ignore(1);
                            fin >> vn;
                            vertex.normal = normals.at(vn - 1);
                        }
                    }
                    vertices.push_back(vertex);
                    indices.push_back(currentIndex++);
                }
                fin.ignore(1024, L'\n');
            }
            else if (0 == wcscmp(command, L"mtllib"))
            {
                wchar_t mtllib[256];
                fin >> mtllib;
                mtlFilenames.push_back(mtllib);
            }
            else if (0 == wcscmp(command, L"usemtl"))
            {
                wchar_t usemtl[MAX_PATH]{ 0 };
                fin >> usemtl;
                subsets.push_back({ usemtl,static_cast<uint32_t>(indices.size()),0 });
            }
            else
            {
                fin.ignore(1024, L'\n');
            }
        }
        std::vector<Subset>::reverse_iterator iterator = subsets.rbegin();
        iterator->indexCount = static_cast<uint32_t>(indices.size()) - iterator->indexStart;
        for (iterator = subsets.rbegin() + 1; iterator != subsets.rend(); ++iterator)
        {
            iterator->indexCount = (iterator - 1)->indexStart - iterator->indexStart;
        }
        fin.close();

        //MTLファイルパーサー部の実装
        std::filesystem::path mtlFilename(objFilename);
        mtlFilename.replace_filename(std::filesystem::path(mtlFilenames[0]).filename());

        fin.open(mtlFilename);
        //_ASSERT_EXPR(fin, L"'MTL file not found.");

        while (fin)
        {
            fin >> command;
            if (0 == wcscmp(command, L"map_Kd"))
            {
                fin.ignore();
                wchar_t mapKd[256];
                fin >> mapKd;

                std::filesystem::path path(objFilename);
                path.replace_filename(std::filesystem::path(mapKd).filename());
                //textureFilename = path;
                materials.rbegin()->textureFilenames[0] = path;
                fin.ignore(1024, L'\n');
            }
            else if (0 == wcscmp(command, L"map_bump") || 0 == wcscmp(command, L"bump"))
            {
                fin.ignore();
                wchar_t mapBump[256];
                fin >> mapBump;
                std::filesystem::path path(objFilename);
                path.replace_filename(std::filesystem::path(mapBump).filename());
                materials.rbegin()->textureFilenames[1] = path;
                fin.ignore(1024, L'\n');
            }
            else if (0 == wcscmp(command, L"newmtl"))
            {
                fin.ignore();
                wchar_t newmtl[256];
                Material material;
                fin >> newmtl;
                material.name = newmtl;
                materials.push_back(material);
            }
            else if (0 == wcscmp(command, L"Kd"))
            {
                float r, g, b;
                fin >> r >> g >> b;
                materials.rbegin()->Kd = { r,g,b,1 };
                fin.ignore(1024, L'\n');
            }
            else
            {
                fin.ignore(1024, L'\n');
            }
        }
        fin.close();



        CreateComBuffers(device, vertices.data(), vertices.size(), indices.data(), indices.size(),
            vertexBuffer.ReleaseAndGetAddressOf(), indexBuffer.ReleaseAndGetAddressOf());

        HRESULT hr{ S_OK };

        D3D11_INPUT_ELEMENT_DESC inputElememtDesc[]
        {
            {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
            D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
            {"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
            D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
            {"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,
            D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0}
        };

        Shader::CreateVSFromCso(device, "./Resources/Shader/StaticMeshVS.cso", vertexShader.GetAddressOf(),
            inputLayout.GetAddressOf(), inputElememtDesc, ARRAYSIZE(inputElememtDesc));
        Shader::CreatePSFromCso(device, "./Resources/Shader/StaticMeshPS.cso", pixelShader.GetAddressOf());

        D3D11_BUFFER_DESC bufferDesc{};
        bufferDesc.ByteWidth = sizeof(Constants);
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffer.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        D3D11_TEXTURE2D_DESC texture2dDesc{};
        /*hr = LoadTextureFromFile(device, textureFilename.c_str(),
            shaderResourceView.GetAddressOf(), &texture2dDesc);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));*/

        //MTLファイルをもたないやつ用
        if (materials.size() == 0)
        {
            for (const Subset& subset : subsets)
            {
                materials.push_back({ subset.usemtl });
            }
            for (Material& material : materials)
            {
                MakeDummyTexture(device, material.shaderResourceViews[0].GetAddressOf(), 0xFFFFFFFF, 16);
                MakeDummyTexture(device, material.shaderResourceViews[1].GetAddressOf(), 0xFFFFFFFF, 16);
            }
        }
        else
        {
            for (Material& material : materials)
            {
                LoadTextureFromFile(device, material.textureFilenames[0].c_str(),
                    material.shaderResourceViews[0].GetAddressOf(), &texture2dDesc);

                //ノーマルマップもってないマテリアル用
                if (material.textureFilenames[1].size() == 0)
                {
                    MakeDummyTexture(device, material.shaderResourceViews[1].GetAddressOf(), 0xFFFFFFFF, 16);
                }
                else
                {
                    LoadTextureFromFile(device, material.textureFilenames[1].c_str(),
                        material.shaderResourceViews[1].GetAddressOf(), &texture2dDesc);
                }
            }
        }

        //バウンディングボックス用メッシュ作成
        CreateBoundingBox(device, max, min);
    }

    void StaticMesh::Render(ID3D11DeviceContext* immediateContext)
    {
        DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) };
        DirectX::XMMATRIX R{ DirectX::XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z) };
        DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(position.x, position.y, position.z) };
        DirectX::XMFLOAT4X4 world;
        DirectX::XMStoreFloat4x4(&world, S * R * T);

        Render(immediateContext, world, color);
    }

    void StaticMesh::Render(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& materialColor)
    {
        if (!renderActive)return;
        uint32_t stride{ sizeof(Vertex) };
        uint32_t offset{ 0 };
        immediateContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
        immediateContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        immediateContext->IASetInputLayout(inputLayout.Get());

        immediateContext->VSSetShader(vertexShader.Get(), nullptr, 0);
        immediateContext->PSSetShader(pixelShader.Get(), nullptr, 0);

        for (const Material& material : materials)
        {
            //シェーダーリソースビューをピクセルシェーダーにバインド
            immediateContext->PSSetShaderResources(0, 1, material.shaderResourceViews[0].GetAddressOf());
            immediateContext->PSSetShaderResources(1, 1, material.shaderResourceViews[1].GetAddressOf());

            Constants data{ world,materialColor };
            immediateContext->UpdateSubresource(constantBuffer.Get(), 0, 0, &data, 0, 0);
            immediateContext->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

            for (const Subset& subset : subsets)
            {
                if (material.name == subset.usemtl)
                {
                    immediateContext->DrawIndexed(subset.indexCount, subset.indexStart, 0);
                }
            }
        }
    }

    void StaticMesh::BoundingBoxRender(ID3D11DeviceContext* immediateContext)
    {
        if (!renderActive)return;
        uint32_t stride{ sizeof(Vertex) };
        uint32_t offset{ 0 };
        DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) };
        DirectX::XMMATRIX R{ DirectX::XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z) };
        DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(position.x, position.y, position.z) };
        DirectX::XMFLOAT4X4 world;
        DirectX::XMStoreFloat4x4(&world, S * R * T);

        immediateContext->IASetVertexBuffers(0, 1, bVertexBuffer.GetAddressOf(), &stride, &offset);
        immediateContext->IASetIndexBuffer(bIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        immediateContext->IASetInputLayout(bInputLayout.Get());

        immediateContext->VSSetShader(bVertexShader.Get(), nullptr, 0);
        immediateContext->PSSetShader(bPixelShader.Get(), nullptr, 0);

        Constants data{ world,DirectX::XMFLOAT4(1,1,1,1) };
        immediateContext->UpdateSubresource(constantBuffer.Get(), 0, 0, &data, 0, 0);
        immediateContext->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

        D3D11_BUFFER_DESC bufferDesc{};
        bIndexBuffer->GetDesc(&bufferDesc);
        immediateContext->DrawIndexed(bufferDesc.ByteWidth / sizeof(uint32_t), 0, 0);
    }

    void StaticMesh::DrawDebug()
    {
        std::string name = "StaticMesh " + std::to_string(myNum);
        ImGui::Begin(name.c_str());

        ImGui::Checkbox("RenderActive", &renderActive);

        bool rV = reverseV ? true : false;
        ImGui::Checkbox("Reverse V", &rV);

        ImGui::DragFloat3("position", &position.x, 0.1f);
        ImGui::DragFloat3("scale", &scale.x, 0.01f);
        ImGui::DragFloat3("angle", &angle.x, 0.01f);
        ImGui::ColorEdit4("color", &color.x);
        ImGui::End();
    }

    void StaticMesh::CreateComBuffers(ID3D11Device* device, Vertex* vertices, size_t vertexCount, uint32_t* indices, size_t indexCount,
        ID3D11Buffer** vertexBuffer, ID3D11Buffer** indexBuffer)
    {
        HRESULT hr{ S_OK };

        D3D11_BUFFER_DESC bufferDesc{};
        D3D11_SUBRESOURCE_DATA subresourceData{};
        bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertexCount);
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 0;
        subresourceData.pSysMem = vertices;
        subresourceData.SysMemPitch = 0;
        subresourceData.SysMemSlicePitch = 0;
        hr = device->CreateBuffer(&bufferDesc, &subresourceData, vertexBuffer);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        bufferDesc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * indexCount);
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        subresourceData.pSysMem = indices;
        hr = device->CreateBuffer(&bufferDesc, &subresourceData, indexBuffer);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }

    void StaticMesh::CreateBoundingBox(ID3D11Device* device, const DirectX::XMFLOAT3 max, const DirectX::XMFLOAT3 min)
    {
        Vertex vertices[24]{};
        //Vertex  vertices[24]{};
        size_t verticesIndex = 24;
        //正立方体のコントロールポイント数は8個、
        //法線の向きが違う頂点が３個あるので頂点情報の総数は8x3 = 24個、
        //頂点情報配列（vertices）にすべて頂点の位置・法線情報を格納する。

        vertices[0] = { {max.x,max.y,min.z},{1,0,0} };
        vertices[1] = { {max.x,max.y,min.z},{0,1,0} };
        vertices[2] = { {max.x,max.y,min.z},{0,0,-1} };

        vertices[3] = { {max.x,max.y,max.z}, {1,0,0} };
        vertices[4] = { {max.x,max.y,max.z}, {0,1,0} };
        vertices[5] = { {max.x,max.y,max.z}, {0,0,1} };

        vertices[6] = { {min.x,max.y,max.z}, {-1,0,0} };
        vertices[7] = { {min.x,max.y,max.z}, {0,1,0} };
        vertices[8] = { {min.x,max.y,max.z}, {0,0,1} };

        vertices[9] = { {min.x,max.y,min.z}, {-1,0,0} };
        vertices[10] = { {min.x,max.y,min.z}, {0,1,0} };
        vertices[11] = { {min.x,max.y,min.z}, {0,0,-1} };


        //こっから下４つ
        vertices[12] = { {max.x,min.y,min.z},{1,0,0} };
        vertices[13] = { {max.x,min.y,min.z},{0,-1,0} };
        vertices[14] = { {max.x,min.y,min.z},{0,0,-1} };

        vertices[15] = { {max.x,min.y,max.z}, {1,0,0} };
        vertices[16] = { {max.x,min.y,max.z}, {0,-1,0} };
        vertices[17] = { {max.x,min.y,max.z}, {0,0,1} };

        vertices[18] = { {min.x,min.y,max.z}, {-1,0,0} };
        vertices[19] = { {min.x,min.y,max.z}, {0,-1,0} };
        vertices[20] = { {min.x,min.y,max.z}, {0,0,1} };

        vertices[21] = { {min.x,min.y,min.z}, {-1,0,0} };
        vertices[22] = { {min.x,min.y,min.z}, {0,-1,0} };
        vertices[23] = { {min.x,min.y,min.z}, {0,0,-1} };

        uint32_t indices[36]{};
        size_t indicesindex = 36;
        //正立方体は６面持ち、１つの面は２つの３角形ポリゴンで構成されるので総数は6x2 = 12個、
        //正立方体を描画するために１２回の３角形ポリゴン描画が必要、よって参照される頂点情報は12x3 = 36回、
        //3角形ポリゴンが参照する頂点情報のインデックス（頂点番号）を描画順に配列（indices）に格納する。
        //時計回りが表面になるように格納すること。

        indices[0] = 2;
        indices[1] = 23;
        indices[2] = 9;

        indices[3] = 2;
        indices[4] = 14;
        indices[5] = 23;


        indices[6] = 0;
        indices[7] = 3;
        indices[8] = 12;

        indices[9] = 3;
        indices[10] = 15;
        indices[11] = 12;


        indices[12] = 5;
        indices[13] = 8;
        indices[14] = 17;

        indices[15] = 8;
        indices[16] = 20;
        indices[17] = 17;


        indices[18] = 6;
        indices[19] = 9;
        indices[20] = 18;

        indices[21] = 9;
        indices[22] = 21;
        indices[23] = 18;


        //こっから上下面
        indices[24] = 7;
        indices[25] = 4;
        indices[26] = 10;

        indices[27] = 4;
        indices[28] = 1;
        indices[29] = 10;


        indices[30] = 16;
        indices[31] = 19;
        indices[32] = 13;

        indices[33] = 19;
        indices[34] = 22;
        indices[35] = 13;

        HRESULT hr{ S_OK };

        CreateComBuffers(device, vertices, verticesIndex, indices, indicesindex,
            bVertexBuffer.ReleaseAndGetAddressOf(), bIndexBuffer.ReleaseAndGetAddressOf());

        D3D11_INPUT_ELEMENT_DESC inputElememtDesc[]
        {
            {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
            D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
            {"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
            D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
        };

        Shader::CreateVSFromCso(device, "./Resources/Shader/GeometricPrimitiveVS.cso", bVertexShader.GetAddressOf(),
            bInputLayout.GetAddressOf(), inputElememtDesc, ARRAYSIZE(inputElememtDesc));
        Shader::CreatePSFromCso(device, "./Resources/Shader/GeometricPrimitivePS.cso", bPixelShader.GetAddressOf());

        D3D11_BUFFER_DESC bufferDesc{};
        bufferDesc.ByteWidth = sizeof(Constants);
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        hr = device->CreateBuffer(&bufferDesc, nullptr, bConstantBuffer.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }
}
