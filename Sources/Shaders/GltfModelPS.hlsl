#include "GltfModel.hlsli"

struct TextureInfo
{
    int index;
    int texcoord;
};
struct NormalTextureInfo
{
    int index;
    int texcoord;
    float scale;
};
struct OcclusionTextureInfo
{
    int index;
    int texcoord;
    float strength;
};
struct PbrMetallicRoughness
{
    float4 basecolorFactor;
    TextureInfo basecolorTexture;
    float metallicFactor;
    float roughnessFactor;
    TextureInfo metallicRoughnessTexture;
};
struct MaterialConstants
{
    float3 emissiveFactor;
    int alphaMode;// "OPAQUE" : 0, "MASK" : 1, "BLEND" : 2
    float alphaCutoff;
    bool doubleSided;
    
    PbrMetallicRoughness pbrMetallicRoughness;
    
    NormalTextureInfo normalTexture;
    OcclusionTextureInfo occlusionTexture;
    TextureInfo emissiveTexture;
};
StructuredBuffer<MaterialConstants> materials : register(t0);

#define BASECOLOR_TEXTURE 0
#define METALLIC_ROUGHNESS_TEXTURE 1
#define NORMAL_TEXTURE 2
#define EMISSIVE_TEXTURE 3
#define OCCLUSION_TEXTURE 4
Texture2D<float4> materialTextures[5] : register(t1);

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState samplerStates[3] : register(s0);

float4 main(VS_OUT pin) : SV_TARGET
{
    MaterialConstants m = materials[material];
    
    float4 basecolor = m.pbrMetallicRoughness.basecolorTexture.index > -1 ?
    materialTextures[BASECOLOR_TEXTURE].Sample(samplerStates[ANISOTROPIC], pin.texcoord) :
    m.pbrMetallicRoughness.basecolorFactor;
    
    float3 emmisive = m.emissiveTexture.index > -1 ?
    materialTextures[EMISSIVE_TEXTURE].Sample(samplerStates[ANISOTROPIC], pin.texcoord).rgb :
    m.emissiveFactor;
    
    float3 N = normalize(pin.wNormal.xyz);
    float3 L = normalize(-lightDirection.xyz);
    
    //return float4(color, 1);
    //float3 color = max(0, dot(N, L)) * m.pbrMetallicRoughness.basecolorFactor.rgb;
    
    float3 color = max(0, dot(N, L)) * basecolor.rgb + emmisive;
    return float4(color, basecolor.a);
    

}