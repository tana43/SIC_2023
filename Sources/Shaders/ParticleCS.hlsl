#include "Particle.hlsli"

RWStructuredBuffer<Particle> particleBuffer : register(u0);

[numthreads(NUMTHREADS_X, 1, 1)]
void main(uint3 dtid : SV_DISPATCHTHREADID)
{
    uint id = dtid.x;
    
    Particle p = particleBuffer[id];
    
    //if (p.age > 10.0)
    //{
    //    const float g = -1.0;
    //    p.velocity.y += g * deltaTime;
    //    p.position += p.velocity * deltaTime;
        
    //    if (p.position.y < 0)
    //    {
    //        p.velocity = 0;
    //        p.position.y = 0;
    //    }
    //}
    
    p.position.y += p.velocity.y * deltaTime;
    
    if (p.position.y > 100)
    {
        p.position.y = -10;
    }
    
    p.age += deltaTime;
    
    particleBuffer[id] = p;
}