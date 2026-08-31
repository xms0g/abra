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
    vec3 TangentViewDir;
} vs_out;

void main() {
    vs_out.TBN = TBN(model * vec4(aTangent, 0.0), normalMatrix * aNormal);
    vec4 worldPos = model * vec4(aPos, 1.0);
    vs_out.WorldPos = worldPos.xyz;
    vs_out.TexCoord = aTexCoord;

    vec3 viewDir = normalize(cameraPos.xyz - vs_out.WorldPos);
    vs_out.TangentViewDir = normalize(transpose(vs_out.TBN) * viewDir);

    gl_Position = projection * view * vec4(vs_out.WorldPos, 1.0);
}
