#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 7) in mat4 aInstanceMatrix;
layout (location = 11) in mat3 aInstanceNormalMatrix;

#include "ub/camera.glsl"
#include "ub/shadow.glsl"
#include "common/TBN.glsl"

out VS_OUT
{
    vec2 TexCoord;
    mat3 TBN;
    vec3 WorldPos;
    vec4 FragPosLightSpace;
    vec3 ViewDir;
    vec3 TangentViewDir;
} vs_out;

void main() {
    vs_out.TexCoord = aTexCoord;
    vs_out.TBN = TBN(aInstanceMatrix * vec4(0.0), aInstanceNormalMatrix * aNormal);

    vec4 worldPos = aInstanceMatrix * vec4(aPos, 1.0);
    vs_out.WorldPos = worldPos.xyz;
    vs_out.ViewDir = normalize(cameraPos.xyz - vs_out.WorldPos);
    vs_out.TangentViewDir = normalize(transpose(vs_out.TBN) * vs_out.ViewDir);
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.WorldPos, 1.0);

    gl_Position = projection * view  * worldPos;
}