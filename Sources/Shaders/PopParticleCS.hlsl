#include "Particle.hlsli"

RWStructuredBuffer<Particle> particleBuffer : register(u0);

[numthreads(NUMTHREADS_X, 1, 1)]
void main(uint3 dtid : SV_DISPATCHTHREADID)
{
    uint id = dtid.x;
    
    Particle p = particleBuffer[id];
    
    if (p.age < 2.0)
    {
        p.position += p.velocity * deltaTime * (2 - p.age);
    }
    
    //p.position.y += p.velocity.y * deltaTime;
    
    p.age += deltaTime;
    
    particleBuffer[id] = p;
}