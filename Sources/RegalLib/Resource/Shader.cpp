#include "Shader.h"
#include <sstream>
#include "../Other/Misc.h"
#include "../Game/Camera.h"
#include "../../../External/imgui/imgui.h"

namespace Regal::Resource
{
    void Shader::CreateVSFromCso(ID3D11Device* device, const char* csoName, ID3D11VertexShader** vertexShader, ID3D11InputLayout** inputLayout, D3D11_INPUT_ELEMENT_DESC* inputElementDesc, SIZE_T BytecodeLength)
    {
        //csoファイルから必要な部分のみ切り抜いて頂点シェーダーを作成する
        HRESULT hr{ S_OK };

        FILE* fp{};
        fopen_s(&fp, csoName, "rb");
        _ASSERT_EXPR_A(fp, "CSO file not found");

        fseek(fp, 0, SEEK_END);
        long csoSize{ ftell(fp) };
        fseek(fp, 0, SEEK_SET);

        std::unique_ptr<unsigned char[]>csoData{ std::make_unique<unsigned char[]>(csoSize) };
        fread(csoData.get(), csoSize, 1, fp);
        fclose(fp);

        hr = device->CreateVertexShader(csoData.get(), csoSize, nullptr, vertexShader);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        if (inputLayout)
        {
            hr = device->CreateInputLayout(inputElementDesc, BytecodeLength,
                csoData.get(), csoSize, inputLayout);
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        }
    }

    void Shader::CreatePSFromCso(ID3D11Device* device, const char* csoName, ID3D11PixelShader** pixelShader)
    {
        HRESULT hr{ S_OK };

        FILE* fp{};
        fopen_s(&fp, csoName, "rb");
        _ASSERT_EXPR_A(fp, "CSO file not found");

        fseek(fp, 0, SEEK_END);
        long csoSize{ ftell(fp) };
        fseek(fp, 0, SEEK_SET);

        std::unique_ptr<unsigned char[]>csoData{ std::make_unique<unsigned char[]>(csoSize) };
        fread(csoData.get(), csoSize, 1, fp);
        fclose(fp);

        hr = device->CreatePixelShader(csoData.get(), csoSize, nullptr, pixelShader);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }

    void Shader::CreateGSFromCso(ID3D11Device* device, const char* csoName, ID3D11GeometryShader** geometryShader)
    {
        FILE* fp = nullptr;
        fopen_s(&fp, csoName, "rb");
        _ASSERT_EXPR_A(fp, "CSO File not found");

        fseek(fp, 0, SEEK_END);
        long csoSz = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        std::unique_ptr<unsigned char[]> csoData = std::make_unique<unsigned char[]>(csoSz);
        fread(csoData.get(), csoSz, 1, fp);
        fclose(fp);

        HRESULT hr = device->CreateGeometryShader(csoData.get(), csoSz, nullptr, geometryShader);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }

    void Shader::CreateCSFromCso(ID3D11Device* device, const char* csoName, ID3D11ComputeShader** computeShader)
    {
        FILE* fp = nullptr;
        fopen_s(&fp, csoName, "rb");
        _ASSERT_EXPR_A(fp, "CSO File not found");

        fseek(fp, 0, SEEK_END);
        long csoSz = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        std::unique_ptr<unsigned char[]> csoData = std::make_unique<unsigned char[]>(csoSz);
        fread(csoData.get(), csoSz, 1, fp);
        fclose(fp);

        HRESULT hr = device->CreateComputeShader(csoData.get(), csoSz, nullptr, computeShader);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }

    Shader::Shader()
    {
    }

    void Shader::CreateSceneBuffer(ID3D11Device* device)
    {
        HRESULT hr{ S_OK };

        D3D11_BUFFER_DESC bufferDesc{};
        bufferDesc.ByteWidth = sizeof(SceneConstants);
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 0;
        hr = device->CreateBuffer(&bufferDesc, nullptr, sceneConstantBuffer.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }

    void Shader::UpdateSceneConstants(ID3D11DeviceContext* immediateContext)
    {
        auto& camera{ Regal::Game::Camera::Instance() };
        camera.UpdateViewProjectionMatrix();

        //定数バッファにセット
        SceneConstants data{};
        DirectX::XMStoreFloat4x4(&data.viewProjection, camera.GetViewProjectionMatrix());
        DirectX::XMMATRIX lightDirection{ DirectX::XMMatrixRotationRollPitchYaw(
            directionalLightAngle.x, 
            directionalLightAngle.y, 
            directionalLightAngle.z) };
        DirectX::XMFLOAT3 front;
        DirectX::XMStoreFloat3(&front, lightDirection.r[2]);
        data.lightDirection = { front.x,front.y,front.z,0 };
        data.cameraPosition = camera.GetPosition();

        immediateContext->UpdateSubresource(sceneConstantBuffer.Get(), 0, 0, &data, 0, 0);
        immediateContext->VSSetConstantBuffers(1, 1, sceneConstantBuffer.GetAddressOf());
        immediateContext->PSSetConstantBuffers(1, 1, sceneConstantBuffer.GetAddressOf());
    }

    void Shader::SceneConstantsDrawDebug()
    {
        Regal::Game::Camera::Instance().DrawDebug();

        if (ImGui::BeginMenu("Light"))
        {
            ImGui::DragFloat3("Angle", &directionalLightAngle.x, 0.01f);
            ImGui::EndMenu();
        }
    }
}