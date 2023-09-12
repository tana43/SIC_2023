cbuffer PARTICLE_CONSTANTS : register(b9)
{
    float3 emitterPosition;
    float particleSize;
    float4 color;
    float time;
    float deltaTime;
};

cbuffer SCENE_CONSTANT_BUFFER : register(b1)
{
    row_major float4x4 viewProjection;
    float4 lightDirection;
    float4 cameraPosition;
};

struct VS_OUT
{
    uint vertexId : VERTEXID;
};
struct GS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};

struct Particle
{
    float4 color;
    float3 position;
    float3 velocity;
    float age;
    int state;
};

#define NUMTHREADS_X 16