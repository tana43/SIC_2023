#include "SkyBoxEffect.hlsli"

VS_OUT main( float4 position : SV_POSITION )
{
    VS_OUT vout;
	
    vout.positionPS = mul(position, WorldViewProj);
    vout.positionPS.z = vout.positionPS.w;
    vout.texcoord = position.xyz;
	
	return vout;
}