#pragma once
#include <d3d11.h>
#include <directxmath.h>
#include <wrl.h>

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
	};
	struct Constants
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4 materialColor;
	};

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

public:
	StaticMesh(ID3D11Device* device, const wchar_t* objWhilename,
		DirectX::XMFLOAT3 pos = { 0,0,0 }, DirectX::XMFLOAT4 color = { 1,1,1,1 });
	virtual ~StaticMesh() = default;

	void Render(ID3D11DeviceContext* immediateContext);
	
	void DrawDebug();

protected:
	void CreateComBuffers(ID3D11Device* device, Vertex* vertices, size_t vertexCount,
		uint32_t* indices, size_t indexCount);

	DirectX::XMFLOAT3 position{ 0,0,0 };
	DirectX::XMFLOAT3 angle{ 0,0,0 };
	DirectX::XMFLOAT3 scale{ 1,1,1 };
	DirectX::XMFLOAT4 color{ 1,1,1,1 };
};

