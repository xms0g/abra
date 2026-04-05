#version 410 core
in VS_OUT
{
    vec2 TexCoord;
    vec3 WorldPos;
    mat3 TBN;
} fs_in;

#include "ub/camera.glsl"
#include "pbr/material.glsl"
#include "common/normalMap.glsl"
#include "pbr/brdf.glsl"

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

vec3 calculateLights(vec3 N, vec3 V, vec3 R, vec3 albedo, float metallic, float roughness, float ao, vec3 worldPos) {
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

    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;
    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;

    vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;

    vec3 color = ambient + Lo;
    color = color / (color + vec3(1.0));

    return color;
}

out vec4 fragColor;

void main() {
    vec4 albedoSample = texture(material.texture_albedo, fs_in.TexCoord);
    if (albedoSample.a < material.alphaCutout) {
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

    vec3 emissive = material.hasEmissiveMap
        ? pow(texture(material.texture_emissive, fs_in.TexCoord).rgb, vec3(2.2))
        : vec3(0.0);

    vec3 N = normal(fs_in.TBN, fs_in.TexCoord, true);
    vec3 V = normalize(viewPos.xyz - fs_in.WorldPos);
    vec3 R = reflect(-V, N);

    vec3 result = calculateLights(N, V, R, albedo, metallic, roughness, ao, fs_in.WorldPos) + emissive;

    fragColor = vec4(result, 1.0);
}