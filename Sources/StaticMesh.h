#pragma once
#include <d3d11.h>
#include <directxmath.h>
#include <wrl.h>
#include <fstream>

#ifdef USE_IMGUI
#include "../imgui/imgui.h"
#endif

class StaticMesh
{
public:
	struct  Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 texcoord;
	};
	struct Constants
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4 materialColor;
	};

	struct subset
	{
		std::wstring usemtl;
		uint32_t indexStart{ 0 };
		uint32_t indexCount{ 0 };
	};

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

public:
	StaticMesh(ID3D11Device* device,const wchar_t* objWhilename, bool reverseV = false,
		DirectX::XMFLOAT3 pos = { 0,0,0 }, DirectX::XMFLOAT4 color = { 1,1,1,1 });
	virtual ~StaticMesh() = default;

	void Render(ID3D11DeviceContext* immediateContext);
	void Render(ID3D11DeviceContext* immediateContext,
		const DirectX::XMFLOAT4X4 & world, const DirectX::XMFLOAT4& materialColor);

	void DrawDebug();

protected:
	void CreateComBuffers(ID3D11Device* device, Vertex* vertices, size_t vertexCount,
		uint32_t* indices, size_t indexCount);

	DirectX::XMFLOAT3 position{ 0,0,0 };
	DirectX::XMFLOAT3 angle{ 0,0,0 };
	DirectX::XMFLOAT3 scale{ 1,1,1 };
	DirectX::XMFLOAT4 color{ 1,1,1,1 };

	std::wstring textureFilename;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;

	bool reverseV;

	std::vector<subset> subsets;
};

