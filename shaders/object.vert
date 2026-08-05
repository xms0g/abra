#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

#include "ub/camera.glsl"
#include "ub/shadow.glsl"
#include "common/TBN.glsl"

uniform mat4 model;
uniform mat3 normalMatrix;

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
    vs_out.TBN = TBN(model, aTangent, normalMatrix, aNormal);
    vs_out.TexCoord = aTexCoord;
    vs_out.WorldPos = vec3(model * vec4(aPos, 1.0));
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.WorldPos, 1.0);
    vs_out.ViewDir = normalize(cameraPos.xyz - vs_out.WorldPos);
    vs_out.TangentViewDir = normalize(transpose(vs_out.TBN) * vs_out.ViewDir);

    gl_Position = projection * view  * model * vec4(aPos, 1.0);
}