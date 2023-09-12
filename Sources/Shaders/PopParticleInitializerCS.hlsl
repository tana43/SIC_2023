#include "Particle.hlsli"

RWStructuredBuffer<Particle> particleBuffer : register(u0);

//float rand(float n)
//{
//    return frac(sin(n) * 43758.5453123);
//}

float rand(float2 co) //引数はシード値と呼ばれる　同じ値を渡せば同じものを返す
{
    return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453);
}

[numthreads(NUMTHREADS_X, 1, 1)]
void main(uint3 dtid : SV_DISPATCHTHREADID)
{
    uint id = dtid.x;
    
    Particle p = particleBuffer[id];
    
    const float noiseScale = 1.0;
    float f0 = rand(float2(id, noiseScale));
    float f1 = rand(float2(f0, noiseScale));
    float f2 = rand(float2(f1, noiseScale));
    
    p.position = emitterPosition;
    
    p.velocity.x = 2.0f * sin(2 * 3.14 * f0);
    p.velocity.y = 2.0f * cos(2 * 3.14 * f0);
    p.velocity.z = 2.0f * f1;
    
    p.color.x = f2 * color.x * 3.0 + color.x;
    p.color.y = f2 * color.y * 3.0 + color.y;
    p.color.z = f2 * color.z * 3.0 + color.z;
    p.color.xyz /= 2.0;
    p.color.w = 1.0f;
    
    //p.age = 10.0 * f2;
    p.age = 1.0 - 2.0 * f2;
    p.state = 0;

    particleBuffer[id] = p;
}