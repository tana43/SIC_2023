#include "FullscreenQuad.hlsli"

#define POINT 0 
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState samplerStates[3] : register(s0);
Texture2D textureMap : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
    //float4 color = textureMaps[0].Sample(samplerStates[ANISOTROPIC], pin.texcoord);
    //float alpha = color.a;
    //color.rgb = smoothstep(0.6, 0.8, dot(color.rgb, float3(0.299, 0.582, 0.144))) * color.rgb;
    //return float4(color.rgb, alpha);
    
    return textureMap.Sample(samplerStates[LINEAR],pin.texcoord);
}