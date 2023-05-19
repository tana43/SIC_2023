#pragma once
#include <d3d11.h>
#include <directxmath.h>
#include <wrl.h>
#include <fstream>
#include <vector>

#ifdef USE_IMGUI
#include "../imgui/imgui.h"
#endif

class StaticMesh
{
	static int num;

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

	struct Subset
	{
		std::wstring usemtl;
		uint32_t indexStart{ 0 };
		uint32_t indexCount{ 0 };
	};

	struct  Material
	{
		std::wstring name;
		DirectX::XMFLOAT4 Ka{0.2f, 0.2f, 0.2f, 1.0f};
		DirectX::XMFLOAT4 Kd{0.8f, 0.8f, 0.8f, 1.0f};
		DirectX::XMFLOAT4 Ks{1.0f, 1.0f, 1.0f, 1.0f};
		std::wstring textureFilenames[2];
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceViews[2];
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
	int myNum;

	void CreateComBuffers(ID3D11Device* device, Vertex* vertices, size_t vertexCount,
		uint32_t* indices, size_t indexCount);

	HRESULT MakeDummyTexture(ID3D11Device* device, ID3D11ShaderResourceView** shaderResourceView,
		DWORD value/*0xAABBGGRR*/, UINT dimension);

	DirectX::XMFLOAT3 position{ 0,0,0 };
	DirectX::XMFLOAT3 angle{ 0,0,0 };
	DirectX::XMFLOAT3 scale{ 1,1,1 };
	DirectX::XMFLOAT4 color{ 1,1,1,1 };

	/*std::wstring textureFilename;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;*/

	bool reverseV;
	
	
	std::vector<Subset> subsets;
	std::vector<Material> materials;
};

