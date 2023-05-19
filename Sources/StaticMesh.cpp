#include "StaticMesh.h"

#include <vector>
#include <string>
#include <filesystem>

#include "Shader.h"
#include "misc.h"
#include "Texture.h"

int StaticMesh::num{ 0 };

StaticMesh::StaticMesh(ID3D11Device* device, const wchar_t* objFilename, bool reverseV, DirectX::XMFLOAT3 pos, DirectX::XMFLOAT4 color)
    : reverseV(reverseV), position(pos), color(color),myNum(num++)
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    uint32_t currentIndex{ 0 };

    std::vector<DirectX::XMFLOAT3> positions;
    std::vector<DirectX::XMFLOAT3> normals;
    std::vector<DirectX::XMFLOAT2> texcoords;
    std::vector<std::wstring> mtlFilenames;

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
        }
        else if (0 == wcscmp(command, L"vn"))
        {
            float i, j, k;
            fin >> i >> j >> k;
            normals.push_back({ i,j,k });
            fin.ignore(1024, L'\n');
        }
        else if (0 == wcscmp(command,L"vt"))
        {
            float u, v; 
            fin >> u >> v;
            if(reverseV)texcoords.push_back({ u,1.0f - v });
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
        else if (0 == wcscmp(command,L"mtllib"))
        {
            wchar_t mtllib[256]; 
            fin >> mtllib;
            mtlFilenames.push_back(mtllib);
        }
        else if (0 == wcscmp(command,L"usemtl"))
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
        if (0 == wcscmp(command,L"map_Kd"))
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
        else if (0 == wcscmp(command,L"map_bump") || 0 == wcscmp(command,L"bump"))
        {
            fin.ignore();
            wchar_t mapBump[256];
            fin >> mapBump;
            std::filesystem::path path(objFilename);
            path.replace_filename(std::filesystem::path(mapBump).filename());
            materials.rbegin()->textureFilenames[1] = path;
            fin.ignore(1024, L'\n');
        }
        else if (0 == wcscmp(command,L"newmtl"))
        {
            fin.ignore();
            wchar_t newmtl[256];
            Material material;
            fin >> newmtl;
            material.name = newmtl;
            materials.push_back(material);
        }
        else if (0 == wcscmp(command,L"Kd"))
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

    

    CreateComBuffers(device, vertices.data(), vertices.size(), indices.data(), indices.size());

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
}

void StaticMesh::Render(ID3D11DeviceContext* immediateContext)
{
    DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(scale.x,scale.y,scale.z) };
    DirectX::XMMATRIX R{ DirectX::XMMatrixRotationRollPitchYaw(angle.x,angle.y,angle.z) };
    DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(position.x,position.y,position.z) };
    DirectX::XMFLOAT4X4 world;
    DirectX::XMStoreFloat4x4(&world, S * R * T);

    Render(immediateContext,world,color);
}

void StaticMesh::Render(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& materialColor)
{
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

void StaticMesh::DrawDebug()
{
    std::string name = "StaticMesh " + std::to_string(myNum);
    ImGui::Begin(name.c_str());

    bool rV = reverseV ? true : false;
    ImGui::Checkbox("Reverse V",&rV);

    ImGui::DragFloat3("position", &position.x, 0.1f);
    ImGui::DragFloat3("scale", &scale.x, 0.01f);
    ImGui::DragFloat3("angle", &angle.x, 0.01f);
    ImGui::ColorEdit4("color", &color.x);
    ImGui::End();
}

void StaticMesh::CreateComBuffers(ID3D11Device* device, Vertex* vertices, size_t vertexCount, uint32_t* indices, size_t indexCount)
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
    hr = device->CreateBuffer(&bufferDesc, &subresourceData, vertexBuffer.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * indexCount);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    subresourceData.pSysMem = indices;
    hr = device->CreateBuffer(&bufferDesc, &subresourceData, indexBuffer.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

HRESULT StaticMesh::MakeDummyTexture(ID3D11Device* device, ID3D11ShaderResourceView** shaderResourceView, DWORD value, UINT dimension)
{
    HRESULT hr{ S_OK };

    D3D11_TEXTURE2D_DESC texture2dDesc{};
    texture2dDesc.Width = dimension;
    texture2dDesc.Height = dimension;
    texture2dDesc.MipLevels = 1;
    texture2dDesc.ArraySize = 1;
    texture2dDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texture2dDesc.SampleDesc.Count = 1;
    texture2dDesc.SampleDesc.Quality = 0;
    texture2dDesc.Usage = D3D11_USAGE_DEFAULT;
    texture2dDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    size_t texels = dimension * dimension;
    std::unique_ptr<DWORD[]> sysmem{std::make_unique<DWORD[]>(texels)};
    for (size_t i = 0; i < texels; ++i)sysmem[i] = value;

    D3D11_SUBRESOURCE_DATA subresourceData;
    subresourceData.pSysMem = sysmem.get();
    subresourceData.SysMemPitch = sizeof(DWORD) * dimension;
    
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
    hr = device->CreateTexture2D(&texture2dDesc, &subresourceData, &texture2d);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
    shaderResourceViewDesc.Format = texture2dDesc.Format;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MipLevels = 1;
    hr = device->CreateShaderResourceView(texture2d.Get(), &shaderResourceViewDesc,
        shaderResourceView);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    return hr;
}

