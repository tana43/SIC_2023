#include "Texture.h"
#include "misc.h"
#include <WICTextureLoader.h>

#include <wrl.h>

#include <string>
#include <map>
#include <memory>

static std::map<std::wstring, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> resources;
HRESULT LoadTextureFromFile(ID3D11Device* device, const wchar_t* filename,
    ID3D11ShaderResourceView** shaderResourceView, D3D11_TEXTURE2D_DESC* texture2dDesc)
{
    HRESULT hr{ S_OK };
    Microsoft::WRL::ComPtr<ID3D11Resource> resource;

    auto it = resources.find(filename);
    if (it != resources.end())
    {
        *shaderResourceView = it->second.Get();
        (*shaderResourceView)->AddRef();
        (*shaderResourceView)->GetResource(resource.GetAddressOf());
    }
    else
    {
        hr = DirectX::CreateWICTextureFromFile(device, filename, resource.GetAddressOf(), shaderResourceView);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        resources.insert(std::make_pair(filename, *shaderResourceView));
    }

    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
    hr = resource.Get()->QueryInterface<ID3D11Texture2D>(texture2d.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    texture2d->GetDesc(texture2dDesc);

    return hr;
}
HRESULT MakeDummyTexture(ID3D11Device* device, ID3D11ShaderResourceView** shaderResourceView, DWORD value, UINT dimension)
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
void ReleaseAllTextures()
{
    resources.clear();
}