#include "SkyBox.hlsli"

VS_OUT main(float4 position : POSITION)
{
    position.z = 1.0f;
    VS_OUT vout;
    vout.position = position;
    
    //ワールド変換
    float4 p = mul(position, inverseViewProjection);
    vout.worldPosition = p / p.w;
	return vout;
}