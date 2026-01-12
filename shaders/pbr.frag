#version 410 core

in vec2 TexCoord;
in vec3 WorldPos;
in vec3 Normal;

#include "ub/camera.glsl"
#include "common/brdf.glsl"

struct Material {
    sampler2D texture_albedo;
    sampler2D texture_normal;
    sampler2D texture_metallic;
    sampler2D texture_roughness;
    sampler2D texture_ao;
};

uniform Material material;

vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(material.texture_normal, TexCoord).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoord);
    vec2 st2 = dFdy(TexCoord);

    vec3 N = normalize(Normal);
    vec3 T = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

out vec4 fragColor;

void main() {
    vec3 albedo = pow(texture(material.texture_albedo, TexCoord).rgb, vec3(2.2));
    float metallic = texture(material.texture_metallic, TexCoord).r;
    float roughness = texture(material.texture_roughness, TexCoord).r;
    float ao = texture(material.texture_ao, TexCoord).r;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(viewPos.xyz - WorldPos);

    vec3 result = calculateLights(albedo, N, metallic, roughness, ao, V);

    fragColor = vec4(result, 1.0);
}