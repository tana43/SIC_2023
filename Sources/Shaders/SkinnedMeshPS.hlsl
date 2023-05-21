#include "SkinnedMesh.hlsli"

float4 main(VS_OUT pin) : SV_TARGET
{
    float3 N = normalize(pin.worldNormal.xyz);
    float3 L = normalize(-lightDirection.xyz);
    float3 diffuse = max(0, dot(N, L));
	return float4(diffuse,1) * pin.color;
}