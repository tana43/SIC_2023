#include "SkinnedMesh.hlsli"

VS_OUT main(VS_IN vin)
{
    VS_OUT vout;
    vout.position = mul(vin.position, mul(world, viewProjection));
    vout.worldPosition = mul(vin.position, world);
    vin.normal.w = 0;
    vout.worldNormal = normalize(mul(vin.normal, world));
    vout.texcoord = vin.texcoord;
    vout.color = materialColor;
	return vout;
}