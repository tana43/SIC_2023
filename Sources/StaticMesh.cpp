#include "StaticMesh.h"

#include <vector>
#include <string>
#include <filesystem>

#include "Shader.h"
#include "misc.h"
#include "Texture.h"

StaticMesh::StaticMesh(ID3D11Device* device, const wchar_t* objFilename,DirectX::XMFLOAT3 pos, DirectX::XMFLOAT4 color)
    : position(pos), color(color)
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    uint32_t currentIndex{ 0 };

    std::vector<DirectX::XMFLOAT3> positions;
    std::vector<DirectX::XMFLOAT3> normals;
    std::vector<DirectX::XMFLOAT2> texcoords;
    std::vector<std::wstring> mtlFilenames;

    std::wifstream fin(objFilename);
    _ASSERT_EXPR(fin, L"'OBJ fole not found.");
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
            texcoords.push_back({ u,1.0f - v });
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
        else
        {
            fin.ignore(1024, L'\n');
        }
    }
    fin.close();

    //MTLファイルパーサー部の実装
    std::filesystem::path mtlFilename(objFilename);
    mtlFilename.replace_filename(std::filesystem::path(mtlFilenames[0]).filename());

    fin.open(mtlFilename);
    _ASSERT_EXPR(fin, L"'MTL file not found.");

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
            textureFilename = path;
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
    hr = LoadTextureFromFile(device, textureFilename.c_str(),
        shaderResourceView.GetAddressOf(), &texture2dDesc);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
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

    //シェーダーリソースビューをピクセルシェーダーにバインド
    immediateContext->PSSetShaderResources(0, 1, shaderResourceView.GetAddressOf());

    Constants data{ world,materialColor };
    immediateContext->UpdateSubresource(constantBuffer.Get(), 0, 0, &data, 0, 0);
    immediateContext->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

    D3D11_BUFFER_DESC bufferDesc{};
    indexBuffer->GetDesc(&bufferDesc);
    immediateContext->DrawIndexed(bufferDesc.ByteWidth / sizeof(uint32_t), 0, 0);
}

void StaticMesh::DrawDebug()
{
    std::string name = "StaticMesh";
    ImGui::Begin(name.c_str());
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

