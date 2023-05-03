#include "Texture.h"
#include "misc.h"
#include <WICTextureLoader.h>

#include <wrl.h>

#include <string>
#include <map>

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
void ReleaseAllTextures()
{
    resources.clear();
}