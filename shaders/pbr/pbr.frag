#version 410 core
in VS_OUT
{
    vec2 TexCoord;
    vec3 WorldPos;
    mat3 TBN;
    vec4 FragPosLightSpace;
} fs_in;

#include "ub/camera.glsl"
#include "ub/light.glsl"
#include "pbr/material.glsl"
#include "common/normalMap.glsl"
#include "pbr/pbr.glsl"

out vec4 fragColor;

void main() {
    vec4 albedoSample = texture(material.texture_albedo, fs_in.TexCoord);
    if (albedoSample.a < material.alphaCutoff) {
        discard;
    }

    vec3 albedo = pow(albedoSample.rgb, vec3(2.2));

    vec3 orm = texture(material.texture_roughnessMetallic, fs_in.TexCoord).rgb;
    float roughness = orm.g;
    float metallic = orm.b;

    float ao = 1.0;
    if (material.hasORM) {
        ao = orm.r;
    } else if (material.hasAOMap) {
        ao = texture(material.texture_ao, fs_in.TexCoord).r;
    }

    vec3 emissive = vec3(0.0);
    if (material.hasEmissiveMap) {
        emissive = pow(texture(material.texture_emissive, fs_in.TexCoord).rgb, vec3(2.2));
    }

    vec3 N = normal(fs_in.TBN, fs_in.TexCoord, true);
    vec3 V = normalize(viewPos.xyz - fs_in.WorldPos);
    vec3 R = reflect(-V, N);

    vec3 result = calculateLights(N, V, R, fs_in.WorldPos, fs_in.FragPosLightSpace, albedo, metallic, roughness, ao) + emissive;

    fragColor = vec4(result, 1.0);
}