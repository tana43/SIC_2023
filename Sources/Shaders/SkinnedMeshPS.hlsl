#include "SkinnedMesh.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2

SamplerState samplerStates[3] : register(s0);
Texture2D textureMaps[4] : register(t0);



float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = textureMaps[0].Sample(samplerStates[ANISOTROPIC], pin.texcoord);
    //float alpha = color.a;
    float alpha = materialColor.a;
    //float3 N = normalize(pin.worldNormal.xyz);
    //float3 T = normalize(pin.worldTangent.xyz);
    //float sigma = pin.worldTangent.w;
    //T = normalize(T - N * dot(N, T));
    //float3 B = normalize(cross(N, T) * sigma);
    
    ////Normal
    //float4 normal = textureMaps[1].Sample(samplerStates[LINEAR], pin.texcoord);
    //normal = (normal * 2.0) - 1.0;
    //N = normalize((normal.x * T) + (normal.y * B) + (normal.z * N));
    
    //float3 L = normalize(-lightDirection.xyz);
    //float3 diffuse = color.rgb * max(0, dot(N, L));
    //float3 V = normalize(cameraPosition.xyz - pin.worldPosition.xyz);
    //float3 specular = pow(max(0, dot(N, normalize(V + L))), 128);
    
    //return float4(diffuse + specular + emissive * /*emissiveIntensity*/10, alpha) * pin.color;
    
    //����̓��C�g�̉e���������S�ɖ�������
    //Emissive
    float3 emissive = textureMaps[2].Sample(samplerStates[LINEAR], pin.texcoord).rgb;
    
    return /*float4(diffuse + specular, alpha) + */color * pin.color + float4(emissive * emissiveIntensity,alpha) * emissiveColor;
}