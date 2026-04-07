#version 410 core
in VS_OUT
{
    vec2 TexCoord;
    vec3 WorldPos;
    mat3 TBN;
    vec3 FragPos;
    vec4 FragPosLightSpace;
} fs_in;

#define ENVIROMENT_INTENSITY 0.2

#include "ub/camera.glsl"
#include "ub/light.glsl"
#include "pbr/material.glsl"
#include "common/normalMap.glsl"
#include "pbr/brdf.glsl"

// Shadow Map
uniform sampler2D shadowMap;
uniform samplerCubeArray shadowCubemap;
uniform sampler2DArray persShadowMap;

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

float calculateDirectionalShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0) {
        return 0.0; // no shadow
    }

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // PCF
    float shadow = 0.0;
    int samples = 2;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    for (int x = -samples; x <= samples; ++x) {
        for (int y = -samples; y <= samples; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= float((2 * samples + 1) * (2 * samples + 1));

    return shadow;
}

vec3 calculateLights(vec3 N, vec3 V, vec3 R, vec3 albedo, float metallic, float roughness, float ao, vec3 worldPos) {
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    for (int i = 0; i < lightCount.x; i++) {
        vec3 lightDir = normalize(-dirLights[i].direction.xyz);
        vec3 lightPos = lightDir * 5.0;
        vec3 radiance = dirLights[i].diffuse.rgb * dirLights[i].intensity;

        float shadow = calculateDirectionalShadow(fs_in.FragPosLightSpace, N, lightDir);

        vec3 reflectance = brdf(lightPos, worldPos, radiance, albedo, N, metallic, roughness, ao, V, F0);

        Lo += reflectance * (1.0 - shadow);
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

    vec3 ambient = (kD * diffuse + specular) * ao * ENVIROMENT_INTENSITY;
    vec3 color = ambient + Lo;

    return color;
}

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

    vec3 result = calculateLights(N, V, R, albedo, metallic, roughness, ao, fs_in.WorldPos) + emissive;

    fragColor = vec4(result, 1.0);
}