#define POINT 0 
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState samplerStates[3] : register(s0);
TextureCube<float4> CubeMap : register(t0);

float4 main(float3 texcoord : TEXCOORD0) : SV_TARGET0
{
    return CubeMap.Sample(samplerStates[LINEAR], normalize(texcoord));
}