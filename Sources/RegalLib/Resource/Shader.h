#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>

namespace Regal::Resource
{
    //ゲープロと同じ作りにしようと思ったけど時間がないのでパス
    class Shader
    {
    public:
        static void CreateVSFromCso(ID3D11Device* device, const char* csoName, ID3D11VertexShader** vertexShader,
            ID3D11InputLayout** inputLayout, D3D11_INPUT_ELEMENT_DESC* inputElementDesc, SIZE_T BytecodeLength);

        static void CreatePSFromCso(ID3D11Device* device, const char* csoName, ID3D11PixelShader** pixelShader);

        static void CreateGSFromCso(ID3D11Device* device, const char* csoName, ID3D11GeometryShader** geometryShader);

        static void CreateCSFromCso(ID3D11Device* device, const char* csoName, ID3D11ComputeShader** computeShader);

    public:
        //Shader() {}
        //virtual ~Shader() {}

        ////描画開始
        //virtual void Begin(ID3D11DeviceContext* immediateContext) = 0;

        ////描画
        //virtual void Draw(ID3D11DeviceContext* immediateContext, const Model* model) = 0;

        ////描画終了
        //virtual void End(ID3D11DeviceContext* immediateContext) = 0;

        struct  SceneConstants
        {
            DirectX::XMFLOAT4X4 viewProjection;	//ビュープロジェクション交換行列
            DirectX::XMFLOAT4 lightDirection;	//ライトの向き
            DirectX::XMFLOAT4 cameraPosition;
            //DirectX::XMFLOAT4X4 inverseViewProjection;//ビュープロジェクション逆行列
        };

        Shader();
        ~Shader() {}

        void CreateSceneBuffer(ID3D11Device* device);

        //シーン用の定数バッファをセット、更新しているだけ
        void UpdateSceneConstants(ID3D11DeviceContext* immediateContext);

        void SceneConstantsDrawDebug();

        Microsoft::WRL::ComPtr<ID3D11Buffer> GetSceneConstanceBuffer() { return sceneConstantBuffer; }

    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> sceneConstantBuffer;

        DirectX::XMFLOAT3 directionalLightAngle{DirectX::XMFLOAT3(0,0,0)};
    };
}
