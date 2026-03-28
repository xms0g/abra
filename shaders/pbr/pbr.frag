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

// IBL
uniform samplerCube irradianceMap;

vec3 calculateLights(vec3 albedo, vec3 N, float metallic, float roughness, float ao, vec3 V, vec3 worldPos) {
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    for (int i = 0; i < lightCount.x; i++) {
        Lo += brdf((-dirLights[i].direction * 5.0).xyz, worldPos, dirLights[i].diffuse.rgb, albedo, N, metallic, roughness, ao, V, F0);
    }
    for (int i = 0; i < lightCount.y; i++) {
        Lo += brdf(pointLights[i].position.xyz, worldPos, pointLights[i].diffuse.rgb, albedo, N, metallic, roughness, ao, V, F0);
    }
    for (int i = 0; i < lightCount.z; i++) {
        Lo += brdf(spotLights[i].position.xyz, worldPos, spotLights[i].diffuse.rgb, albedo, N, metallic, roughness, ao, V, F0);
    }

    vec3 kS = fresnelSchlick(max(dot(N, V), 0.0), F0);
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;
    vec3 ambient = (kD * diffuse) * ao;

    vec3 color = ambient + Lo;

    return color;
}

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