#include "SkinnedMesh.hlsli"

VS_OUT main(VS_IN vin)
{
    vin.normal.w = 0;

    float4 blendedPosition = { 0, 0, 0, 1 };
    float4 blendedNormal = { 0, 0, 0, 0 };
    for (int boneIndex = 0; boneIndex < 4; ++boneIndex)
    {
        blendedPosition += vin.boneWeights[boneIndex] * mul(vin.position, boneTransforms[vin.boneIndices[boneIndex]]);
        blendedNormal += vin.boneWeights[boneIndex] * mul(vin.normal, boneTransforms[vin.boneIndices[boneIndex]]);
    }
    vin.position = float4(blendedPosition.xyz, 1.0f);
    vin.normal = float4(blendedNormal.xyz, 0.0f);
    
    VS_OUT vout;
    vout.position = mul(vin.position, mul(world, viewProjection));
    
    vout.worldPosition = mul(vin.position, world);
    
    vout.worldNormal = normalize(mul(vin.normal, world));
    
    vout.texcoord = vin.texcoord;
    
    #if 1
    vout.color = materialColor;
    #else
    vout.color = 0;
    const float4 boneColors[4] =
    {
        { 1, 0, 0, 1 },
        { 0, 1, 0, 1 },
        { 0, 0, 1, 1 },
        { 1, 1, 1, 1 },
    };
    for (int boneIndex = 0; boneIndex < 4; ++boneIndex)
    {
        vout.color += boneColors[vin.boneIndices[boneIndex] % 4]
        * vin.boneWeights[boneIndex];
    }
    #endif
    
    return vout;
}