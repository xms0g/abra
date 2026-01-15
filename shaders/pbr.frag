#version 410 core
in VS_OUT
{
    vec2 TexCoord;
    vec3 WorldPos;
    mat3 TBN;
} fs_in;

#include "ub/camera.glsl"
#include "common/pbrMaterial.glsl"
#include "common/normalMap.glsl"
#include "common/brdf.glsl"

out vec4 fragColor;

void main() {
    vec3 albedo = pow(texture(material.texture_albedo, fs_in.TexCoord).rgb, vec3(2.2));
    float metallic = texture(material.texture_metallic, fs_in.TexCoord).r;
    float roughness = texture(material.texture_roughness, fs_in.TexCoord).r;
    vec3 emissive = material.hasEmissiveMap ? pow(texture(material.texture_emissive, fs_in.TexCoord).rgb, vec3(2.2)) : vec3(0.0);
    float ao = texture(material.texture_ao, fs_in.TexCoord).r;

    vec3 N = normal(fs_in.TBN, fs_in.TexCoord, true);
    vec3 V = normalize(viewPos.xyz - fs_in.WorldPos);

    vec3 result = calculateLights(albedo, N, metallic, roughness, ao, V, fs_in.WorldPos) + emissive;

    fragColor = vec4(result, 1.0);
}