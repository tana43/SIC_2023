#include "SkinnedMesh.hlsli"

VS_OUT main(VS_IN vin)
{
    VS_OUT vout;
    vout.position = mul(vin.position, mul(world, viewProjection));
    
    vout.worldPosition = mul(vin.position, world);
    vin.normal.w = 0;
    vout.worldNormal = normalize(mul(vin.normal, world));
    
    vout.texcoord = vin.texcoord;
    
    #if 0
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