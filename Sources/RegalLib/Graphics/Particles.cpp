#include "Particles.h"

#include "../Resource/Shader.h"
#include "../Other/Misc.h"

#include "../../../External/imgui/imgui.h"

using namespace Regal::Resource;

namespace Regal::Graphics
{
    Particles::Particles(ID3D11Device* device, size_t particleCount) : maxParticleCount(particleCount)
    {
        HRESULT hr{ S_OK };
        D3D11_BUFFER_DESC bufferDesc{};
        bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Particle) * particleCount);
        bufferDesc.StructureByteStride = sizeof(Particle);
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        hr = device->CreateBuffer(&bufferDesc, NULL, particleBuffer.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
        shaderResourceViewDesc.Format = DXGI_FORMAT_UNKNOWN;
        shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        shaderResourceViewDesc.Buffer.ElementOffset = 0;
        shaderResourceViewDesc.Buffer.NumElements = static_cast<UINT>(particleCount);
        hr = device->CreateShaderResourceView(particleBuffer.Get(), &shaderResourceViewDesc, particleBufferSrv.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        D3D11_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc;
        unorderedAccessViewDesc.Format = DXGI_FORMAT_UNKNOWN;
        unorderedAccessViewDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        unorderedAccessViewDesc.Buffer.FirstElement = 0;
        unorderedAccessViewDesc.Buffer.NumElements = static_cast<UINT>(particleCount);
        unorderedAccessViewDesc.Buffer.Flags = 0;
        hr = device->CreateUnorderedAccessView(particleBuffer.Get(), &unorderedAccessViewDesc, particleBufferUav.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        bufferDesc.ByteWidth = sizeof(ParticleConstants);
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 0;
        hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffer.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        Shader::CreateVSFromCso(device, "./Resources/Shader/ParticleVS.cso", particleVS.ReleaseAndGetAddressOf(), NULL, NULL, 0);
        Shader::CreatePSFromCso(device, "./Resources/Shader/ParticlePS.cso", particlePS.ReleaseAndGetAddressOf());
        Shader::CreateGSFromCso(device, "./Resources/Shader/ParticleGS.cso", particleGS.ReleaseAndGetAddressOf());
        Shader::CreateCSFromCso(device, "./Resources/Shader/ParticleCS.cso", particleCS.ReleaseAndGetAddressOf());
        Shader::CreateCSFromCso(device, "./Resources/Shader/ParticleInitializerCS.cso", particleInitializerCS.ReleaseAndGetAddressOf());
    }

    UINT Align(UINT num, UINT alignment)
    {
        return (num + (alignment - 1)) & ~(alignment - 1);
    }

    void Particles::Integrate(ID3D11DeviceContext* immediateContext, float deltaTime)
    {
        immediateContext->CSSetUnorderedAccessViews(0, 1, particleBufferUav.GetAddressOf(), NULL);

        particleData.time += deltaTime;
        particleData.deltaTime = deltaTime;
        immediateContext->UpdateSubresource(constantBuffer.Get(), 0, 0, &particleData, 0, 0);
        immediateContext->CSSetConstantBuffers(9, 1, constantBuffer.GetAddressOf());

        immediateContext->CSSetShader(particleCS.Get(), NULL, 0);

        const UINT threadGroupCountX = Align(static_cast<UINT>(maxParticleCount), NUMTHREADS_X) / NUMTHREADS_X;
        immediateContext->Dispatch(threadGroupCountX, 1, 1);

        ID3D11UnorderedAccessView* nullUnorederedAccessView{};
        immediateContext->CSSetUnorderedAccessViews(0, 1, &nullUnorederedAccessView, NULL);
    }

    void Particles::Initialize(ID3D11DeviceContext* immediateContext, float deltaTime)
    {
        immediateContext->CSSetUnorderedAccessViews(0, 1, particleBufferUav.GetAddressOf(), NULL);

        particleData.time += deltaTime;
        particleData.deltaTime = deltaTime;
        immediateContext->UpdateSubresource(constantBuffer.Get(), 0, 0, &particleData, 0, 0);
        immediateContext->CSSetConstantBuffers(9, 1, constantBuffer.GetAddressOf());

        immediateContext->CSSetShader(particleInitializerCS.Get(), NULL, 0);

        const UINT threadGroupCountX = Align(static_cast<UINT>(maxParticleCount), NUMTHREADS_X) / NUMTHREADS_X;
        immediateContext->Dispatch(threadGroupCountX, 1, 1);

        ID3D11UnorderedAccessView* nullUnorderedAccessView{};
        immediateContext->CSSetUnorderedAccessViews(0, 1, &nullUnorderedAccessView, NULL);
    }

    void Particles::Render(ID3D11DeviceContext* immediateContext)
    {
        immediateContext->VSSetShader(particleVS.Get(), NULL, 0);
        immediateContext->PSSetShader(particlePS.Get(), NULL, 0);
        immediateContext->GSSetShader(particleGS.Get(), NULL, 0);
        immediateContext->GSSetShaderResources(9, 1, particleBufferSrv.GetAddressOf());

        immediateContext->UpdateSubresource(constantBuffer.Get(), 0, 0, &particleData, 0, 0);
        immediateContext->VSSetConstantBuffers(9, 1, constantBuffer.GetAddressOf());
        immediateContext->PSSetConstantBuffers(9, 1, constantBuffer.GetAddressOf());
        immediateContext->GSSetConstantBuffers(9, 1, constantBuffer.GetAddressOf());

        immediateContext->IASetInputLayout(NULL);
        immediateContext->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
        immediateContext->IASetIndexBuffer(NULL, DXGI_FORMAT_R32_UINT, 0);
        immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
        immediateContext->Draw(static_cast<UINT>(maxParticleCount), 0);

        ID3D11ShaderResourceView* nullShaderResourceView{};
        immediateContext->GSSetShaderResources(9, 1, &nullShaderResourceView);
        immediateContext->VSSetShader(NULL, NULL, 0);
        immediateContext->PSSetShader(NULL, NULL, 0);
        immediateContext->GSSetShader(NULL, NULL, 0);
    }

    void Particles::DrawDebug()
    {
        ImGui::Begin("Particle System");

        ImGui::SliderFloat("Particle Size", &particleData.particleSize, 0.0f, 0.03f);
        ImGui::DragFloat3("Emitter Position", &particleData.emitterPosition.x, 0.1f);
        //ImGui::SliderFloat("Effect Time", &particleData.time,0.0f, 20.0f);

        ImGui::End();
    }
}
