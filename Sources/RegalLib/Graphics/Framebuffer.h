#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <cstdint>

namespace Regal::Graphics
{
    class Framebuffer
    {
    public:
        Framebuffer(ID3D11Device* device, uint32_t width, uint32_t height, DXGI_FORMAT format, bool useDepth, bool useStencil = false);
        virtual ~Framebuffer() = default;

        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceViews[2];
        D3D11_VIEWPORT viewport;

        void Clear(ID3D11DeviceContext* immediateContext,
            float r = 0, float g = 0, float b = 0, float a = 1, float depth = 1);
        void Activate(ID3D11DeviceContext* immediateContext);
        void Deactivate(ID3D11DeviceContext* immediateContext);

    private:
        UINT viewportCount{ D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE };
        D3D11_VIEWPORT cachedViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> cachedRenderTargetView;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> cachedDepthStencilView;
    };
}

