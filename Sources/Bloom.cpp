#include "Bloom.h"

#include "../imgui/imgui.h"

#include <vector>

#include "Shader.h"
#include "misc.h"

Bloom::Bloom(ID3D11Device* device, uint32_t width, uint32_t height)
{
    bitBlockTransfer = std::make_unique<FullscreenQuad>(device);

    glowExtraction = std::make_unique<Framebuffer>(device, width, height);
    for (size_t downsampledIndex = 0; downsampledIndex < downsampledCount; downsampledIndex++)
    {
        gaussianBlur[downsampledIndex][0] = std::make_unique<Framebuffer>(device, width >> downsampledIndex, height >> downsampledIndex);
        gaussianBlur[downsampledIndex][1] = std::make_unique<Framebuffer>(device, width >> downsampledIndex, height >> downsampledIndex);
    }
    Shader::CreatePSFromCso(device, "./Resources/Shader/GlowExtractionPS.cso", glowExtractionPs.GetAddressOf());
    Shader::CreatePSFromCso(device, "./Resources/Shader/GaussianBlurDownsamplingPS.cso", gaussianBlurDownsamplingPs.GetAddressOf());
    Shader::CreatePSFromCso(device, "./Resources/Shader/GaussianBlurHorizontalPS.cso", gaussianBlurHorizontalPs.GetAddressOf());
    Shader::CreatePSFromCso(device, "./Resources/Shader/GaussianBlurVerticalPS.cso", gaussianBlurVerticalPs.GetAddressOf());
    Shader::CreatePSFromCso(device, "./Resources/Shader/GaussianBlurUpsamplingPS.cso", gaussianBlurUpsamplingPs.GetAddressOf());

    HRESULT hr{ S_OK };

    D3D11_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.DepthBias = 0;
    rasterizerDesc.DepthBiasClamp = 0;
    rasterizerDesc.SlopeScaledDepthBias = 0;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.ScissorEnable = FALSE;
    rasterizerDesc.MultisampleEnable = FALSE;
    rasterizerDesc.AntialiasedLineEnable = FALSE;
    hr = device->CreateRasterizerState(&rasterizerDesc, rasterizerState.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    D3D11_BLEND_DESC blendDesc{};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    blendDesc.RenderTarget[0].BlendEnable = FALSE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    hr = device->CreateBlendState(&blendDesc, blendState.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.ByteWidth = sizeof(BloomConstants);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;
    hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void Bloom::Make(ID3D11DeviceContext* immediateContext, ID3D11ShaderResourceView* colorMap)
{
    ID3D11ShaderResourceView* nullShaderResourceView{};
    ID3D11ShaderResourceView* cachedShaderResourceViews[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
    immediateContext->PSGetShaderResources(0, downsampledCount, cachedShaderResourceViews);

    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> cachedDepthStencilState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> cachedRasterizerState;
    Microsoft::WRL::ComPtr<ID3D11BlendState> cachedBlendState;
    FLOAT blendFactor[4];
    UINT sampleMask;
    immediateContext->OMGetDepthStencilState(cachedDepthStencilState.GetAddressOf(), 0);
    immediateContext->RSGetState(cachedRasterizerState.GetAddressOf());
    immediateContext->OMGetBlendState(cachedBlendState.GetAddressOf(),blendFactor,&sampleMask);

    Microsoft::WRL::ComPtr<ID3D11Buffer> cachedConstantBuffer;
    immediateContext->PSGetConstantBuffers(8, 1, cachedConstantBuffer.GetAddressOf());

    //ステートをバインド
    immediateContext->OMSetDepthStencilState(depthStencilState.Get(), 0);
    immediateContext->RSSetState(rasterizerState.Get());
    immediateContext->OMSetBlendState(blendState.Get(), nullptr, 0xFFFFFFFF);

    //定数バッファ更新
    BloomConstants data{};
    data.bloomExtractionThreshold = bloomExtractionThreshold;
    data.bloomIntensity = bloomIntensity;
    immediateContext->UpdateSubresource(constantBuffer.Get(), 0, 0, &data, 0, 0);
    immediateContext->PSSetConstantBuffers(8, 1, constantBuffer.GetAddressOf());

    //輝度成分抽出
    glowExtraction->Clear(immediateContext, 0, 0, 0, 1);
    glowExtraction->Activate(immediateContext);
    bitBlockTransfer->Bilt(immediateContext, &colorMap, 0, 1, glowExtractionPs.Get());
    glowExtraction->Deactivate(immediateContext);
    immediateContext->PSSetShaderResources(0, 1, &nullShaderResourceView);

    //ガウシアンブラー：画像処理においてガウス関数をもちいて画像をぼかす処理
    //リニアサンプラーによるガウシアンブラー
    gaussianBlur[0][0]->Clear(immediateContext, 0, 0, 0, 1);
    gaussianBlur[0][0]->Activate(immediateContext);
    bitBlockTransfer->Bilt(immediateContext, glowExtraction->shaderResourceViews[0].GetAddressOf(), 0, 1, 
        gaussianBlurDownsamplingPs.Get());
    gaussianBlur[0][0]->Deactivate(immediateContext);
    immediateContext->PSSetShaderResources(0, 1, &nullShaderResourceView);

    //Ping-pong gaussian blur
    //水平方向、垂直方向にブラーを掛けてるっぽい
    gaussianBlur[0][1]->Clear(immediateContext, 0, 0, 0, 1);
    gaussianBlur[0][1]->Activate(immediateContext);
    bitBlockTransfer->Bilt(immediateContext, gaussianBlur[0][0]->shaderResourceViews[0].GetAddressOf(), 0, 1, 
        gaussianBlurHorizontalPs.Get());
    gaussianBlur[0][1]->Deactivate(immediateContext);
    immediateContext->PSSetShaderResources(0, 1, &nullShaderResourceView);

    gaussianBlur[0][0]->Clear(immediateContext, 0, 0, 0, 1);
    gaussianBlur[0][0]->Activate(immediateContext);
    bitBlockTransfer->Bilt(immediateContext, gaussianBlur[0][1]->shaderResourceViews[0].GetAddressOf(),0,1,
        gaussianBlurVerticalPs.Get());
    gaussianBlur[0][0]->Deactivate(immediateContext);
    immediateContext->PSSetShaderResources(0, 1, &nullShaderResourceView);

    for (size_t downsampledIndex = 1; downsampledIndex < downsampledCount; ++downsampledIndex)
    {
        //Downsampling
        gaussianBlur[downsampledIndex][0]->Clear(immediateContext, 0, 0, 0, 1);
        gaussianBlur[downsampledIndex][0]->Activate(immediateContext);
        bitBlockTransfer->Bilt(immediateContext, gaussianBlur[downsampledIndex - 1][0]->shaderResourceViews->GetAddressOf(), 0, 1,
            gaussianBlurDownsamplingPs.Get());
        gaussianBlur[downsampledIndex][0]->Deactivate(immediateContext);
        immediateContext->PSSetShaderResources(0, 1, &nullShaderResourceView);

        //Ping-pong gaussian blur
        gaussianBlur[downsampledIndex][1]->Clear(immediateContext, 0, 0, 0, 1);
        gaussianBlur[downsampledIndex][1]->Activate(immediateContext);
        bitBlockTransfer->Bilt(immediateContext, gaussianBlur[downsampledIndex][0]->shaderResourceViews->GetAddressOf(), 0, 1,
            gaussianBlurHorizontalPs.Get());
        gaussianBlur[downsampledIndex][1]->Deactivate(immediateContext);
        immediateContext->PSSetShaderResources(0, 1, &nullShaderResourceView);

        gaussianBlur[downsampledIndex][0]->Clear(immediateContext, 0, 0, 0, 1);
        gaussianBlur[downsampledIndex][0]->Activate(immediateContext);
        bitBlockTransfer->Bilt(immediateContext, gaussianBlur[downsampledIndex - 1][0]->shaderResourceViews->GetAddressOf(), 0, 1,
            gaussianBlurVerticalPs.Get());
        gaussianBlur[downsampledIndex][0]->Deactivate(immediateContext);
        immediateContext->PSSetShaderResources(0, 1, &nullShaderResourceView);
    }


    //Downsampling
    glowExtraction->Clear(immediateContext, 0, 0, 0, 1);
    glowExtraction->Activate(immediateContext);
    std::vector<ID3D11ShaderResourceView*> shaderResourceViews;
    for (size_t downsampledIndex = 0; downsampledIndex < downsampledCount; ++downsampledIndex)
    {
        shaderResourceViews.push_back(gaussianBlur[downsampledIndex][0]->shaderResourceViews[0].Get());
    }
    bitBlockTransfer->Bilt(immediateContext, shaderResourceViews.data(), 0, downsampledCount, gaussianBlurUpsamplingPs.Get());
    glowExtraction->Deactivate(immediateContext);
    immediateContext->PSSetShaderResources(0, 1, &nullShaderResourceView);

    //ステートを元に戻す
    immediateContext->PSSetConstantBuffers(8, 1, cachedConstantBuffer.GetAddressOf());

    immediateContext->OMSetDepthStencilState(cachedDepthStencilState.Get(), 0);
    immediateContext->RSSetState(cachedRasterizerState.Get());
    immediateContext->OMSetBlendState(cachedBlendState.Get(), blendFactor, sampleMask);

    immediateContext->PSSetShaderResources(0, downsampledCount, cachedShaderResourceViews);
    for (ID3D11ShaderResourceView* cachedShaderResourceView : cachedShaderResourceViews)
    {
        if (cachedShaderResourceView)cachedShaderResourceView->Release();
    }
}

void Bloom::DrawDebug()
{
    if (ImGui::TreeNode("Bloom"))
    {
        ImGui::SliderFloat("Threshold", &bloomExtractionThreshold, 0.0f, 5.0f);
        ImGui::SliderFloat("Intensity", &bloomIntensity, 0.0f, 5.0f);

        ImGui::TreePop();
    }
}
