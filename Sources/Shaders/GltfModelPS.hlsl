#include "GltfModel.hlsli"
#include "BidirectionalReflectanceDistributionFunction.hlsli"

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
    int alphaMode; // "OPAQUE" : 0, "MASK" : 1, "BLEND" : 2
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
    const float GAMMA = 2.2;
    
    const MaterialConstants m = materials[material];
    
    //なんかtexcoordのyが１だけ大きくなってたからゴリ押しで引いた
    pin.texcoord.y -= 1;

    //float4 basecolor = m.pbrMetallicRoughness.basecolorTexture.index > -1 ?
    //materialTextures[BASECOLOR_TEXTURE].Sample(samplerStates[ANISOTROPIC], pin.texcoord) :
    //m.pbrMetallicRoughness.basecolorFactor;
    
    float4 basecolorFactor = m.pbrMetallicRoughness.basecolorFactor;
    const int basecolorTexture = m.pbrMetallicRoughness.basecolorTexture.index;
    if (basecolorTexture > -1)
    {
        float4 sampled = materialTextures[BASECOLOR_TEXTURE].Sample(samplerStates[ANISOTROPIC], pin.texcoord);
        sampled.rgb = pow(sampled.rgb, GAMMA);
        basecolorFactor *= sampled;
    }
    
    //    float3 emmisive = m.emissiveTexture.index > -1 ?
    //materialTextures[EMISSIVE_TEXTURE].Sample(samplerStates[ANISOTROPIC], pin.texcoord).rgb :
    //m.emissiveFactor;
    
    float3 emmisiveFactor = m.emissiveFactor;
    const int emmisiveTexture = m.emissiveTexture.index;
    if (emmisiveTexture > -1)
    {
        float4 sampled = materialTextures[EMISSIVE_TEXTURE].Sample(samplerStates[ANISOTROPIC], pin.texcoord);
        sampled.rgb = pow(sampled.rgb, GAMMA);
        emmisiveFactor *= sampled.rgb;
    }
    
    float roughnessFactor = m.pbrMetallicRoughness.roughnessFactor;
    float metallicFactor = m.pbrMetallicRoughness.metallicFactor;
    const int metallicRoughnessTexture = m.pbrMetallicRoughness.metallicRoughnessTexture.index;
    if (metallicRoughnessTexture > -1)
    {
        float4 sampled = materialTextures[METALLIC_ROUGHNESS_TEXTURE].Sample(samplerStates[LINEAR],
        pin.texcoord);
        roughnessFactor *= sampled.g;
        metallicFactor *= sampled.b;
    }
    
    float occlusionFactor = 1.0;
    const int occlusionTexture = m.occlusionTexture.index;
    if (occlusionTexture > -1)
    {
        float4 sampled = materialTextures[OCCLUSION_TEXTURE].Sample(samplerStates[LINEAR], pin.texcoord);
        occlusionFactor *= sampled.r;
    }
    const float occlusionStrengh = m.occlusionTexture.strength;
    
    const float3 f0 = lerp(0.04, basecolorFactor.rgb, metallicFactor);
    const float3 f90 = 1.0;
    const float alphaRoughness = roughnessFactor * roughnessFactor;
    const float3 cDiff = lerp(basecolorFactor.rgb, 0.0, metallicFactor);
    
    const float3 P = pin.wPosition.xyz;
    const float3 V = normalize(cameraPosition.xyz - pin.wPosition.xyz);
    
    float3 N = normalize(pin.wNormal.xyz);
    float3 T = hasTangent ? normalize(pin.wTangent.xyz) : float3(1, 0, 0);
    float sigma = hasTangent ? pin.wTangent.w : 1.0;
    T = normalize(T - N * dot(N, T));
    float3 B = normalize(cross(N, T) * sigma);
    
    const int normalTexture = m.normalTexture.index;
    if(normalTexture > -1)
    {
        float4 sampled = materialTextures[NORMAL_TEXTURE].Sample(samplerStates[LINEAR], pin.texcoord);
        float3 normalFactor = sampled.xyz;
        normalFactor = (normalFactor * 2.0) - 1.0;
        normalFactor = normalize(normalFactor * float3(m.normalTexture.scale, m.normalTexture.scale, 1.0));
        N = normalize((normalFactor.x * T) + (normalFactor.y * B) + (normalFactor.z * N));
    }
    
    float3 diffuse = 0;
    float3 specular = 0;
    
    //各ライトのシェーディング処理のループ
    float3 L = normalize(-lightDirection.xyz);
    float3 Li = float3(1.0, 1.0, 1.0);//ライト角度
    const float NoL = max(0.0, dot(N, L));
    const float NoV = max(0.0, dot(N, V));
    if (NoL > 0.0 || NoV > 0.0)
    {
        const float3 R = reflect(-L, N);
        const float3 H = normalize(V + L);
        
        const float NoH = max(0.0, dot(N, H));
        const float HoV = max(0.0, dot(H, V));

        diffuse += Li * NoL * BrdfLambertian(f0, f90, cDiff, HoV);
        specular += Li * NoL * BrdfSpecularGgx(f0, f90, alphaRoughness, HoV, NoL, NoV, NoH);
    }
    
    float3 emmisive = emmisiveFactor;
    diffuse = lerp(diffuse, diffuse * occlusionFactor, occlusionStrengh);
    specular = lerp(specular, specular * occlusionFactor, occlusionStrengh);
    
    float3 Lo = diffuse + specular + emmisive;
    return float4(Lo, basecolorFactor.a);
    
    
    //return float4(color, 1);
    //float3 color = max(0, dot(N, L)) * m.pbrMetallicRoughness.basecolorFactor.rgb;
    
    //    float3 color = max(0, dot(N, L)) * basecolor.rgb + emmisive.rgb;
    //return float4(color, basecolor.a);
    

}