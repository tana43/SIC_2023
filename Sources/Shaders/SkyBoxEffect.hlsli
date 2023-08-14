cbuffer SkyboxConstants : register(b0)
{
    float4x4 WorldViewProj;
}

struct VS_OUT
{
    float3 texcoord : TEXCOORD0;
    float4 positionPS : SV_POSITION;
};