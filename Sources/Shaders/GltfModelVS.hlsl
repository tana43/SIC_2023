#include "GltfModel.hlsli"

VS_OUT main(VS_IN vin)
{
    VS_OUT vout;
	
    vin.position.w = 1;
    vout.position = mul(vin.position, mul(world, viewProjection));
    vout.wPosition = mul(vin.position, world);
    
    vin.normal.w = 0;
    vout.wNormal = normalize(mul(vin.normal, world));
    
    float sigma = vin.tangent.w;
    vin.tangent.w = 0;
    vout.wTangent = normalize(mul(vin.tangent, world));
    vout.wTangent = sigma;
	
    vout.texcoord = vin.texcoord;
    
    return vout;
}