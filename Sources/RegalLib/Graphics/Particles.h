#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>

#include <vector>

#define NUMTHREADS_X 16

namespace Regal::Graphics
{
    struct Particle
    {
        DirectX::XMFLOAT4 color{1, 1, 1, 1};
        DirectX::XMFLOAT3 position{0, 0, 0};
        DirectX::XMFLOAT3 velocity{0, 0, 0};
        float age{};
        int state{};
    };

    class Particles
    {
    public:
        const size_t maxParticleCount;
        struct  ParticleConstants
        {
            DirectX::XMFLOAT3 emitterPosition{};
            float particleSize{ 0.08f };
            DirectX::XMFLOAT4 color;
            float time{};
            float deltaTime{};
            float something[2];

        };
        ParticleConstants particleData;

        Microsoft::WRL::ComPtr<ID3D11Buffer> particleBuffer;
        Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> particleBufferUav;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleBufferSrv;

        Microsoft::WRL::ComPtr<ID3D11VertexShader> particleVS;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> particlePS;
        Microsoft::WRL::ComPtr<ID3D11GeometryShader> particleGS;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
        Microsoft::WRL::ComPtr<ID3D11ComputeShader> particleCS;
        Microsoft::WRL::ComPtr<ID3D11ComputeShader> particleInitializerCS;
        Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

        Particles(ID3D11Device* device, size_t particleCount,
            const char* cs = "./Resources/Shader/ParticleCS.cso",
            const char* initializeCs = "./Resources/Shader/ParticleInitializerCS.cso");
        Particles(const Particles&) = delete;
        Particles& operator=(const Particles&) = delete;
        Particles(Particles&&) noexcept = delete;
        Particles& operator=(Particles&&) noexcept = delete;
        virtual ~Particles() = default;

        //çXêVèàóù
        void Integrate(ID3D11DeviceContext* immediateContext, float deltaTime);


        void Initialize(ID3D11DeviceContext* immediateContext, float deltaTime);
        void Render(ID3D11DeviceContext* immediateContext);
        void DrawDebug();

        void SetColor(DirectX::XMFLOAT4 col);

        DirectX::XMFLOAT4 color{1.0f,1.0f,1.0f,1.0f};

    protected:
        virtual void SetComputeShader(ID3D11Device* device,const char* cs,const char* initializeCs);
    };

    class PopParticles : public Particles
    {
    public:
        PopParticles(ID3D11Device* device, size_t particleCount) : 
            Particles(device, particleCount,
                "./Resources/Shader/PopParticleCS.cso",
                "./Resources/Shader/PopParticleInitializerCS.cso") {}
    };
}
