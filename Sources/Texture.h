#pragma once
#include <d3d11.h>

HRESULT LoadTextureFromFile(ID3D11Device* device, const wchar_t* filename,
    ID3D11ShaderResourceView** shaderResourceView, D3D11_TEXTURE2D_DESC* texture2dDesc);

void ReleaseAllTextures();

//aaa