#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;

#include "ub/camera.glsl"
#include "common/TBN.glsl"

uniform mat4 model;
uniform mat3 normalMatrix;

out VS_OUT
{
    vec2 TexCoord;
    vec3 WorldPos;
    mat3 TBN;
} vs_out;

void main()
{
    vs_out.TexCoord = aTexCoord;
    vs_out.WorldPos = vec3(model * vec4(aPos, 1.0));
    vs_out.TBN = TBN(model, aTangent, normalMatrix, aNormal);

    gl_Position =  projection * view * vec4(vs_out.WorldPos, 1.0);
}