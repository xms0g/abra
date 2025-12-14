in VS_OUT
{
    vec2 TexCoord;
    mat3 TBN;
    vec3 FragPos;
    vec4 FragPosLightSpace;
    vec3 ViewDir;
    vec3 TangentViewDir;
} fs_in;

#include "ub/camera.glsl"
#include "ub/shadow.glsl"
#include "common/material.glsl"
#include "common/blinnPhong.glsl"
#include "common/normalMap.glsl"
#include "common/parallaxMap.glsl"
