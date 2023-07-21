#include "GltfModel.hlsli"

float4 main(VS_OUT pin) : SV_TARGET
{
    float3 N = normalize(pin.wNormal.xyz);
    float3 L = normalize(-lightDirection.xyz);
    
    float3 color = max(0, dot(N, L));
    return float4(color, 1);
}