struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 worldPosition : POSITION;
};

cbuffer SCENE_CONSTANT_BUFFER : register(b1)
{
    row_major float4x4 inverseViewProjection;
    float4 cameraPosition;
}

static const float PI = 3.141592654f;