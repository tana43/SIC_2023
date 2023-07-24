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
    float3 emissicveFactor;
    int alphaMode;// "OPAQUE" : 0, "MASK" : 1, "BLEND" : 2
    float alphaCutoff;
    bool doubleSided;
    
    PbrMetallicRoughness pbrMetallicRoughness;
    
    NormalTextureInfo normalTexture;
    OcclusionTextureInfo occlusionTexture;
    TextureInfo emissiveTexture;
};
StructuredBuffer<MaterialConstants> materials : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
    MaterialConstants m = materials[material];
    
    float3 N = normalize(pin.wNormal.xyz);
    float3 L = normalize(-lightDirection.xyz);
    
    float3 color = max(0, dot(N, L)) * m.pbrMetallicRoughness.basecolorFactor.rgb;
    return float4(color, 1);
}