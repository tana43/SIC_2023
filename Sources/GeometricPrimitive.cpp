#include "GeometricPrimitive.h"
#include "misc.h"
#include <sstream>
#include "Shader.h"

GeometricPrimitive::GeometricPrimitive(ID3D11Device* device)
{
    Vertex  vertices[24]{};
    //³—§•û‘Ì‚ÌƒRƒ“ƒgƒ[ƒ‹ƒ|ƒCƒ“ƒg”‚Í8ŒÂA
    //–@ü‚ÌŒü‚«‚ªˆá‚¤’¸“_‚ª‚RŒÂ‚ ‚é‚Ì‚Å’¸“_î•ñ‚Ì‘”‚Í8x3 = 24ŒÂA
    //’¸“_î•ñ”z—ñiverticesj‚É‚·‚×‚Ä’¸“_‚ÌˆÊ’uE–@üî•ñ‚ðŠi”[‚·‚éB

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


    //‚±‚Á‚©‚ç‰º‚S‚Â
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

    uint32_t indices[36]{};
    //³—§•û‘Ì‚Í‚U–ÊŽ‚¿A‚P‚Â‚Ì–Ê‚Í‚Q‚Â‚Ì‚RŠpŒ`ƒ|ƒŠƒSƒ“‚Å\¬‚³‚ê‚é‚Ì‚Å‘”‚Í6x2 = 12ŒÂA
    //³—§•û‘Ì‚ð•`‰æ‚·‚é‚½‚ß‚É‚P‚Q‰ñ‚Ì‚RŠpŒ`ƒ|ƒŠƒSƒ“•`‰æ‚ª•K—vA‚æ‚Á‚ÄŽQÆ‚³‚ê‚é’¸“_î•ñ‚Í12x3 = 36‰ñA
    //3ŠpŒ`ƒ|ƒŠƒSƒ“‚ªŽQÆ‚·‚é’¸“_î•ñ‚ÌƒCƒ“ƒfƒbƒNƒXi’¸“_”Ô†j‚ð•`‰æ‡‚É”z—ñiindicesj‚ÉŠi”[‚·‚éB
    //ŽžŒv‰ñ‚è‚ª•\–Ê‚É‚È‚é‚æ‚¤‚ÉŠi”[‚·‚é‚±‚ÆB

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


    //‚±‚Á‚©‚çã‰º–Ê
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

    CreateComBuffers(device, vertices, 24, indices, 36);

    HRESULT hr{ S_OK };

    D3D11_INPUT_ELEMENT_DESC inputElememtDesc[]
    {
        {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
        D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
        D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
    };
    
    Shader::CreateVSFromCso(device, "../Resources/Shader/GeometricPrimitiveVS.cso", vertexShader.GetAddressOf(),
        inputLayout.GetAddressOf(), inputElememtDesc, ARRAYSIZE(inputElememtDesc));
    Shader::CreatePSFromCso(device, "../Resources/Shader/GeometricPrimitivePS.cso", pixelShader.GetAddressOf());
    
    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.ByteWidth = sizeof(Constants);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void GeometricPrimitive::Render(ID3D11DeviceContext* immediateContext)
{
    uint32_t stride{ sizeof(Vertex) };
    uint32_t offset{ 0 };
    immediateContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
    immediateContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    immediateContext->IASetInputLayout(inputLayout.Get());

    immediateContext->VSSetShader(vertexShader.Get(), nullptr, 0);
    immediateContext->PSSetShader(pixelShader.Get(), nullptr, 0);

    DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(scale.x,scale.y,scale.z) };
    DirectX::XMMATRIX R{ DirectX::XMMatrixRotationRollPitchYaw(angle.x,angle.y,angle.z) };
    DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(position.x,position.y,position.z) };
    DirectX::XMFLOAT4X4 world;
    DirectX::XMStoreFloat4x4(&world, S * R * T);

    Constants data{ world,color};
    immediateContext->UpdateSubresource(constantBuffer.Get(), 0, 0, &data, 0, 0);
    immediateContext->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

    D3D11_BUFFER_DESC bufferDesc{};
    indexBuffer->GetDesc(&bufferDesc);
    immediateContext->DrawIndexed(bufferDesc.ByteWidth / sizeof(uint32_t), 0, 0);
}

void GeometricPrimitive::DrawDebug()
{
    if (ImGui::TreeNode("GeometricPrimitive"))
    {   
        ImGui::DragFloat3("position", &position.x,0.1f);
        ImGui::DragFloat3("scale", &scale.x,0.01f);
        ImGui::DragFloat3("angle", &angle.x,0.01f);
        ImGui::ColorEdit4("color", &color.x);
        ImGui::TreePop();
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
    _ASSERT_EXPR(SUCCEEDED(hr),hr_trace(hr));

    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * indexCount);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    subresourceData.pSysMem = indices;
    hr = device->CreateBuffer(&bufferDesc, &subresourceData, indexBuffer.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr),hr_trace(hr));
}
