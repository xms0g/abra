#ifndef LEGACY_OBJECT_GLSL
#define LEGACY_OBJECT_GLSL

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
#include "legacy/material.glsl"
#include "legacy/blinnPhong.glsl"
#include "common/normalMap.glsl"
#include "common/parallaxMap.glsl"

#endif