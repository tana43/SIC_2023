#include "GeometricPrimitive.h"
#include "../Other/Misc.h"
#include <sstream>
#include "../Resource/Shader.h"

#define _USE_MATH_DEFINES
#include <math.h>

namespace Regal::Resource
{
    int GeometricPrimitive::num{ 0 };

    GeometricPrimitive::GeometricPrimitive(ID3D11Device* device, MeshType meshType, DirectX::XMFLOAT3 pos, DirectX::XMFLOAT4 color)
        : myNum(num++), position(pos), color(color)
    {

        //std::unique_ptr<Vertex> vertices;
        //std::unique_ptr<uint32_t> indices;
        Vertex* vertices{};
        uint32_t* indices{};

        switch (meshType)
        {
        case GeometricPrimitive::MeshType::Cube:
            vertices = new Vertex[24];
            indices = new uint32_t[36];
            ShapeCubeMesh(vertices, indices);
            break;

        case GeometricPrimitive::MeshType::Sphere:
            break;

        case GeometricPrimitive::MeshType::Cylinder:
            vertices = new Vertex[98];
            indices = new uint32_t[192];
            ShapeCylinderMesh(vertices, indices);
            break;
        }


        CreateComBuffers(device, vertices, verticesIndex, indices, indicesindex);

        HRESULT hr{ S_OK };

        D3D11_INPUT_ELEMENT_DESC inputElememtDesc[]
        {
            {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
            D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
            {"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
            D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
        };

        Shader::CreateVSFromCso(device, "./Resources/Shader/GeometricPrimitiveVS.cso", vertexShader.GetAddressOf(),
            inputLayout.GetAddressOf(), inputElememtDesc, ARRAYSIZE(inputElememtDesc));
        Shader::CreatePSFromCso(device, "./Resources/Shader/GeometricPrimitivePS.cso", pixelShader.GetAddressOf());

        D3D11_BUFFER_DESC bufferDesc{};
        bufferDesc.ByteWidth = sizeof(Constants);
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffer.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        delete[] vertices;
        delete[] indices;
    }

    void GeometricPrimitive::Render(ID3D11DeviceContext* immediateContext)
    {
        if (!renderActive)return;
        DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) };
        DirectX::XMMATRIX R{ DirectX::XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z) };
        DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(position.x, position.y, position.z) };
        DirectX::XMFLOAT4X4 world;
        DirectX::XMStoreFloat4x4(&world, S * R * T);

        Render(immediateContext, world, color);
    }

    void GeometricPrimitive::Render(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& materialColor)
    {
        uint32_t stride{ sizeof(Vertex) };
        uint32_t offset{ 0 };
        immediateContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
        immediateContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        immediateContext->IASetInputLayout(inputLayout.Get());

        immediateContext->VSSetShader(vertexShader.Get(), nullptr, 0);
        immediateContext->PSSetShader(pixelShader.Get(), nullptr, 0);

        Constants data{ world,materialColor };
        immediateContext->UpdateSubresource(constantBuffer.Get(), 0, 0, &data, 0, 0);
        immediateContext->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

        D3D11_BUFFER_DESC bufferDesc{};
        indexBuffer->GetDesc(&bufferDesc);
        immediateContext->DrawIndexed(bufferDesc.ByteWidth / sizeof(uint32_t), 0, 0);
    }

    void GeometricPrimitive::DrawDebug()
    {
        std::string name = "GeometricPrimitive " + std::to_string(myNum);
        ImGui::Begin(name.c_str());

        ImGui::Checkbox("RenderActive", &renderActive);
        ImGui::DragFloat3("position", &position.x, 0.1f);
        ImGui::DragFloat3("scale", &scale.x, 0.01f);
        ImGui::DragFloat3("angle", &angle.x, 0.01f);
        ImGui::ColorEdit4("color", &color.x);
        ImGui::End();

    }

    void GeometricPrimitive::ShapeCubeMesh(Vertex* vertices, uint32_t* indices)
    {
        //Vertex  vertices[24]{};
        verticesIndex = 24;
        //正立方体のコントロールポイント数は8個、
        //法線の向きが違う頂点が３個あるので頂点情報の総数は8x3 = 24個、
        //頂点情報配列（vertices）にすべて頂点の位置・法線情報を格納する。

        vertices[0] = { {0.5f,0.5f,-0.5f},{1,0,0} };
        vertices[1] = { {0.5f,0.5f,-0.5f},{0,1,0} };
        vertices[2] = { {0.5f,0.5f,-0.5f},{0,0,-1} };

        vertices[3] = { {0.5f,0.5f,0.5f}, {1,0,0} };
        vertices[4] = { {0.5f,0.5f,0.5f}, {0,1,0} };
        vertices[5] = { {0.5f,0.5f,0.5f}, {0,0,1} };

        vertices[6] = { {-0.5f,0.5f,0.5f}, {-1,0,0} };
        vertices[7] = { {-0.5f,0.5f,0.5f}, {0,1,0} };
        vertices[8] = { {-0.5f,0.5f,0.5f}, {0,0,1} };

        vertices[9] = { {-0.5f,0.5f,-0.5f}, {-1,0,0} };
        vertices[10] = { {-0.5f,0.5f,-0.5f}, {0,1,0} };
        vertices[11] = { {-0.5f,0.5f,-0.5f}, {0,0,-1} };


        //こっから下４つ
        vertices[12] = { {0.5f,-0.5f,-0.5f},{1,0,0} };
        vertices[13] = { {0.5f,-0.5f,-0.5f},{0,-1,0} };
        vertices[14] = { {0.5f,-0.5f,-0.5f},{0,0,-1} };

        vertices[15] = { {0.5f,-0.5f,0.5f}, {1,0,0} };
        vertices[16] = { {0.5f,-0.5f,0.5f}, {0,-1,0} };
        vertices[17] = { {0.5f,-0.5f,0.5f}, {0,0,1} };

        vertices[18] = { {-0.5f,-0.5f,0.5f}, {-1,0,0} };
        vertices[19] = { {-0.5f,-0.5f,0.5f}, {0,-1,0} };
        vertices[20] = { {-0.5f,-0.5f,0.5f}, {0,0,1} };

        vertices[21] = { {-0.5f,-0.5f,-0.5f}, {-1,0,0} };
        vertices[22] = { {-0.5f,-0.5f,-0.5f}, {0,-1,0} };
        vertices[23] = { {-0.5f,-0.5f,-0.5f}, {0,0,-1} };

        //uint32_t indices[36]{};

        indicesindex = 36;
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
    }

    void GeometricPrimitive::ShapeSphereMesh(Vertex* vertices, uint32_t* indices)
    {

    }

    void GeometricPrimitive::ShapeCylinderMesh(Vertex* vertices, uint32_t* indices)
    {
        verticesIndex = 98;
        float pi = static_cast<float>(M_PI);
        float radian = -pi;
        float r = 0.5f;
        for (int i = 0; i < 6 * 16; i += 6)
        {
            vertices[i] = { {sinf(radian) * r,0.5f,cosf(radian) * r},{sinf(radian),0,cosf(radian)} };
            vertices[i + 1] = { {sinf(radian) * r,0.5f,cosf(radian) * r},{sinf(radian + (pi * 2.0f) / 8.0f),0,cosf(radian + (pi * 2.0f) / 8.0f)} };
            vertices[i + 2] = { {sinf(radian) * r,0.5f,cosf(radian) * r},{0,1,0} };

            vertices[i + 3] = { {sinf(radian) * r,-0.5f,cosf(radian) * r},{sinf(radian),0,cosf(radian)} };
            vertices[i + 4] = { {sinf(radian) * r,-0.5f,cosf(radian) * r},{sinf(radian + (pi * 2.0f) / 8.0f),0,cosf(radian + (pi * 2.0f) / 8.0f)} };
            vertices[i + 5] = { {sinf(radian) * r,-0.5f,cosf(radian) * r},{0,-1,0} };


            radian += (pi * 2) / 16;
        }
        //上下の中心点
        vertices[96] = { {0,0.5f,0},{0,1,0} };
        vertices[97] = { {0,-0.5f,0},{0,-1,0} };

        indicesindex = 192;
        for (int i = 0; i < 16; i++)
        {
            int j = i * 12;
            int k = i * 6;

            if (i == 15)
            {
                //つなぎ目部分
                indices[0 + j] = 4 + k;
                indices[1 + j] = 0;
                indices[2 + j] = 1 + k;

                indices[3 + j] = 4 + k;
                indices[4 + j] = 3;
                indices[5 + j] = 0;

                indices[6 + j] = 2 + k;
                indices[7 + j] = 2;
                indices[8 + j] = 96;

                indices[9 + j] = 97;
                indices[10 + j] = 5;
                indices[11 + j] = 5 + k;
                break;
            }

            indices[0 + j] = 4 + k;
            indices[1 + j] = 6 + k;
            indices[2 + j] = 1 + k;

            indices[3 + j] = 4 + k;
            indices[4 + j] = 9 + k;
            indices[5 + j] = 6 + k;

            indices[6 + j] = 2 + k;
            indices[7 + j] = 8 + k;
            indices[8 + j] = 96;

            indices[9 + j] = 97;
            indices[10 + j] = 11 + k;
            indices[11 + j] = 5 + k;
        }
    }

    void GeometricPrimitive::CreateComBuffers(ID3D11Device* device, Vertex* vertices, size_t vertexCount, uint32_t* indices, size_t indexCount)
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
}