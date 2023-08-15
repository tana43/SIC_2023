struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 worldPosition : POSITION;
};

cbuffer SCENE_CONSTANT_BUFFER : register(b1)
{
    row_major float4x4 viewProjection;
    float4 lightDirection;
    float4 cameraPosition;
    row_major float4x4 inverseViewProjection;
}

static const float PI = 3.141592654f;