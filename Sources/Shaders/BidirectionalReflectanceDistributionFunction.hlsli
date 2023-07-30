#ifndef __BIDIRECTIONAL_REFLECTANCE_DISTRIBUTION_FUNCTION_HLSL__
#define __BIDIRECTIONAL_REFLECTANCE_DISTRIBUTION_FUNCTION_HLSL__

float3 FSchlick(float3 f0,float3 f90,float VoH)
{
    return f0 + (f90 - f0) * pow(clamp(1.0 - VoH, 0.0, 1.0), 5.0);
}
float Vggx(float NoL, float NoV, float alphaRoughness)
{
    float alphaRoughnessSq = alphaRoughness * alphaRoughness;
    
    float ggxv = NoL * sqrt(NoV * NoV * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);
    float ggxl = NoV * sqrt(NoL * NoL * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);

    float ggx = ggxv + ggxl;
    return (ggx > 0.0) ? 0.5 / ggx : 0.0;
}
float Dggx(float NoH,float alphaRoughness)
{
    const float PI = 3.14159265358979;
    float alphaRoughnessSq = alphaRoughness * alphaRoughness;
    float f = (NoH * NoH) * (alphaRoughnessSq - 1.0) + 1.0;
    return alphaRoughnessSq / (PI * f * f);
}
float3 BrdfLambertian(float3 f0,float3 f90,float3 diffuseColor,float VoH)
{
    const float PI = 3.14159265358979;
    return (1.0 - FSchlick(f0, f90, VoH)) * (diffuseColor / PI);
}
float3 BrdfSpecularGgx(float3 f0,float3 f90,float alphaRoughness,
float VoH,float NoL,float NoV,float NoH)
{
    float3 F = FSchlick(f0, f90, VoH);
    float Vis = Vggx(NoL, NoV, alphaRoughness);
    float D = Dggx(NoH, alphaRoughness);
    
    return F * Vis * D;
}

#endif