#include "Particle.hlsli"

RWStructuredBuffer<Particle> particleBuffer : register(u0);

float rand(float n)
{
    return frac(sin(n) * 43758.5453123);
}

[numthreads(NUMTHREADS_X, 1, 1)]
void main(uint3 dtid : SV_DISPATCHTHREADID)
{
    uint id = dtid.x;
    
    Particle p = particleBuffer[id];
    
    const float noiseScale = 1.0;
    float f0 = rand(id * noiseScale);
    float f1 = rand(f0 * noiseScale);
    float f2 = rand(f1 * noiseScale);
    
    p.position = emitterPosition;
    
    //p.velocity.x = 1.0 * sin(2 * 3.14 * f0);
    //p.velocity.y = 2.0 * f1;
    //p.velocity.z = 1.0 * cos(2 * 3.14 * f0);
    
    p.position.y += f0 * other[0];
    
    //p.velocity.x = 2.0 * f1;
    p.velocity.x = other[1] * f1;
    
    p.color.z = f0 * color.z;
    p.color.y = f0 * color.y;
    p.color.x = f0 * color.x;
    p.color.xyz *= 3.0;
    p.color.w = 1.0f;
    
    p.age = 10.0 * f2;
    p.state = 0;

    particleBuffer[id] = p;
}

