#include "FrameBuffer.h"
#include "misc.h"

Framebuffer::Framebuffer(ID3D11Device* device, uint32_t width, uint32_t height)
{
    HRESULT hr{ S_OK };

    Microsoft::WRL::ComPtr<ID3D11Texture2D> renderTargetBuffer;
    D3D11_TEXTURE2D_DESC texture2dDesc{};
    texture2dDesc.Width = width;
    texture2dDesc.Height = height;
    texture2dDesc.MipLevels = 1;
    texture2dDesc.ArraySize = 1;
    texture2dDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    texture2dDesc.SampleDesc.Count = 1;
    texture2dDesc.SampleDesc.Quality = 0;
    texture2dDesc.Usage = D3D11_USAGE_DEFAULT;
    texture2dDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    texture2dDesc.CPUAccessFlags = 0;
    texture2dDesc.MiscFlags = 0;
    hr = device->CreateTexture2D(&texture2dDesc, 0, renderTargetBuffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
    renderTargetViewDesc.Format = texture2dDesc.Format;
    renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    hr = device->CreateRenderTargetView(renderTargetBuffer.Get(), &renderTargetViewDesc,
        renderTargetView.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
    shaderResourceViewDesc.Format = texture2dDesc.Format;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MipLevels = 1;
    hr = device->CreateShaderResourceView(renderTargetBuffer.Get(), &shaderResourceViewDesc,
        shaderResourceViews[0].GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
    texture2dDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    texture2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    hr = device->CreateTexture2D(&texture2dDesc, 0, depthStencilBuffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Flags = 0;
    hr = device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilViewDesc,
        depthStencilView.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    hr = device->CreateShaderResourceView(depthStencilBuffer.Get(), &shaderResourceViewDesc,
        shaderResourceViews[1].GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;

    /*8: Microsoft::WRL::ComPtr<ID3D11Texture2D> render_target_buffer;
9: D3D11_TEXTURE2D_DESC texture2d_desc{};
10: texture2d_desc.Width = width;
11: texture2d_desc.Height = height;
12: texture2d_desc.MipLevels = 1;
13: texture2d_desc.ArraySize = 1;
14: texture2d_desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
15: texture2d_desc.SampleDesc.Count = 1;
16: texture2d_desc.SampleDesc.Quality = 0;
17: texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
18: texture2d_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
19: texture2d_desc.CPUAccessFlags = 0;
20: texture2d_desc.MiscFlags = 0;
21: hr = device->CreateTexture2D(&texture2d_desc, 0, render_target_buffer.GetAddressOf());
22: _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
23: 
24: D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc{};
25: render_target_view_desc.Format = texture2d_desc.Format;
26: render_target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
27: hr = device->CreateRenderTargetView(render_target_buffer.Get(), &render_target_view_desc, 
28: render_target_view.GetAddressOf());
29: _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
30: 
31: D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc{};
32: shader_resource_view_desc.Format = texture2d_desc.Format;
33: shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
34: shader_resource_view_desc.Texture2D.MipLevels = 1;
35: hr = device->CreateShaderResourceView(render_target_buffer.Get(), &shader_resource_view_desc, 
36: shader_resource_views[0].GetAddressOf());
37: _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
38: 
39: Microsoft::WRL::ComPtr<ID3D11Texture2D> depth_stencil_buffer;
40: texture2d_desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
41: texture2d_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
42: hr = device->CreateTexture2D(&texture2d_desc, 0, depth_stencil_buffer.GetAddressOf());
43: _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
44: 
45: D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc{};
46: depth_stencil_view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
47: depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
48: depth_stencil_view_desc.Flags = 0;
49: hr = device->CreateDepthStencilView(depth_stencil_buffer.Get(), &depth_stencil_view_desc, 
50: depth_stencil_view.GetAddressOf());
51: _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
52: 
53: shader_resource_view_desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
54: shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
55: hr = device->CreateShaderResourceView(depth_stencil_buffer.Get(), &shader_resource_view_desc, 
56: shader_resource_views[1].GetAddressOf());
57: _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
58: 
59: viewport.Width = static_cast<float>(width);
60: viewport.Height = static_cast<float>(height);
61: viewport.MinDepth = 0.0f;
62: viewport.MaxDepth = 1.0f;
63: viewport.TopLeftX = 0.0f;
64: viewport.TopLeftY = 0.0f;
*/
}

void Framebuffer::Clear(ID3D11DeviceContext* immediateContext, float r, float g, float b, float a, float depth)
{
    float color[4]{ r,g,b,a };
    immediateContext->ClearRenderTargetView(renderTargetView.Get(), color);
    immediateContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, depth, 0);
}

void Framebuffer::Activate(ID3D11DeviceContext* immediateContext)
{
    viewportCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    immediateContext->RSGetViewports(&viewportCount, cachedViewports);
    immediateContext->OMGetRenderTargets(1, cachedRenderTargetView.ReleaseAndGetAddressOf(),
        cachedDepthStencilView.ReleaseAndGetAddressOf());

    immediateContext->RSSetViewports(1, &viewport);
    immediateContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(),
        depthStencilView.Get());

    /*viewport_count = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
76: immediate_context->RSGetViewports(&viewport_count, cached_viewports);
77: immediate_context->OMGetRenderTargets(1, cached_render_target_view.ReleaseAndGetAddressOf(), 
78: cached_depth_stencil_view.ReleaseAndGetAddressOf());
79: 
80: immediate_context->RSSetViewports(1, &viewport);
81: immediate_context->OMSetRenderTargets(1, render_target_view.GetAddressOf(), 
82: depth_stencil_view.Get());*/
}

void Framebuffer::Deactivate(ID3D11DeviceContext* immediateContext)
{
    immediateContext->RSSetViewports(viewportCount, cachedViewports);
    immediateContext->OMSetRenderTargets(1, cachedRenderTargetView.GetAddressOf(),
        cachedDepthStencilView.Get());

    /*immediate_context->RSSetViewports(viewport_count, cached_viewports);
87: immediate_context->OMSetRenderTargets(1, cached_render_target_view.GetAddressOf(), 
88: cached_depth_stencil_view.Get());*/
}
